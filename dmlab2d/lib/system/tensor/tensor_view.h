// Copyright (C) 2016-2019 The DMLab2D Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DMLAB2D_LIB_SYSTEM_TENSOR_TENSOR_VIEW_H_
#define DMLAB2D_LIB_SYSTEM_TENSOR_TENSOR_VIEW_H_

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#include <ostream>
#include <random>
#include <utility>
#include <vector>

#include "Eigen/Dense"

namespace deepmind::lab2d {
namespace tensor {

using ShapeVector = std::vector<std::size_t>;
using StrideVector = std::vector<std::ptrdiff_t>;

// Implementation of std::exclusive_scan.
template <typename T, typename It, typename ItOut, typename BinaryOp>
ItOut ExclusiveScan(It first, It last, ItOut result, T init, BinaryOp op) {
  for (; first != last; ++first) {
    *result++ = init;
    init = op(init, *first);
  }
  return result;
}

// Class for calculating offsets into storage for a tensor.
// Supports functions which do not require manipulation of the storage data.
// Can have any stride but the default is to have strides in row-major
// (C-style) order.
class Layout {
 public:
  // Constructs a layout with the shape and stride such that:
  // ..., shape[n - 2] * shape[n - 1], shape[n - 1], 1
  // So if shape is {3, 4, 5} - stride is {20, 5, 1}.
  explicit Layout(ShapeVector shape) : shape_(std::move(shape)), offset_(0) {
    stride_.reserve(shape_.size());
    ExclusiveScan(shape_.rbegin(), shape_.rend(), std::back_inserter(stride_),
                  1, std::multiplies<std::ptrdiff_t>{});
    std::reverse(stride_.begin(), stride_.end());
  }

  // Returns the index corresponding to the `flat_index` in `shape` if visited
  // in Layout order and `flat_index` is within bounds. Otherwise returns
  // std::nullopt.
  //
  // Examples:
  //   UnravelIndex(ShapeVector{3, 4}, 0) => {0, 0}
  //   UnravelIndex(ShapeVector{3, 4}, 2) => {0, 2}
  //   UnravelIndex(ShapeVector{3, 4}, 4) => {1, 0}
  //   UnravelIndex(ShapeVector{3, 4}, 9) => {2, 1}
  //   UnravelIndex(ShapeVector{3, 4}, 12) => std::nullopt
  static std::optional<ShapeVector> UnravelIndex(const ShapeVector& shape,
                                                 std::size_t flat_index) {
    std::optional<ShapeVector> result;
    if (std::size_t max_index = num_elements(shape); flat_index < max_index) {
      result.emplace(shape.size());
      std::transform(shape.begin(), shape.end(), result->begin(),
                     [&max_index, &flat_index](std::size_t s) {
                       max_index /= s;
                       size_t index = flat_index / max_index;
                       flat_index -= index * max_index;
                       return index;
                     });
    }
    return result;
  }

  // Transposes dim0 and dim1.
  // This changes the way data is iterated, effectively transposing the tensor
  // this is a layout of.
  // If dim0 and dim1 are valid, it transposes the layout in those dimensions
  // and returns true, otherwise it returns false.
  bool Transpose(std::size_t dim0, std::size_t dim1) {
    if (dim0 < shape_.size() && dim1 < shape_.size()) {
      std::swap(shape_[dim0], shape_[dim1]);
      std::swap(stride_[dim0], stride_[dim1]);
      return true;
    } else {
      return false;
    }
  }

  // Selects an index of a dimension in a layout.
  // If dim and index are invalid the routine returns false.
  //
  // If the shape.size() greater than one, the tensor rank is reduced by one,
  // hiding entries other than index in that dimension.
  //
  // If the shape.size() is 1, the layout is set around that index.
  //
  // Returns whether the routine was successful.
  //
  // Example:
  // view = { { 1, 2 },
  //          { 3, 4 } };
  // view.Select(1, 1) == true
  // view == {2, 4}
  // view.Select(0, 1) == true
  // view == {4}
  // view.Select(0, 1) == false
  bool Select(std::size_t dim, std::size_t index) {
    if (dim < shape_.size() && index < shape_[dim]) {
      offset_ += stride_[dim] * index;
      shape_.erase(shape_.begin() + dim);
      stride_.erase(stride_.begin() + dim);
      return true;
    } else {
      return false;
    }
  }

  // Narrows the dimension 'dim' of the tensor layout.
  // If the arguments are valid: the dimension dim is narrowed to
  // [index, index + size - 1].
  // Returns whether the arguments are valid.
  bool Narrow(std::size_t dim, std::size_t index, std::size_t size) {
    if (dim < shape_.size() && index < shape_[dim] &&
        size + index <= shape_[dim]) {
      offset_ += stride_[dim] * index;
      shape_[dim] = size;
      return true;
    } else {
      return false;
    }
  }

  // Reverses the dimension 'dim' of the tensor layout. If the 'dim' is valid
  // the indexing of that dimension is reversed.
  // Returns whether dim is valid.
  //
  // Example:
  // view = { { 1, 2, 3 },
  //          { 4, 5, 6 } };
  // view.Reverse(0) == true
  // view == { { 4, 5, 6 },
  //           { 1, 2, 3 } };
  //
  // view = { { 1, 2, 3 },
  //          { 4, 5, 6 } };
  // view.Reverse(1) == true
  // view == { { 3, 2, 1 },
  //           { 6, 5, 4 }};
  //
  // view.Reverse(2) == false
  bool Reverse(std::size_t dim) {
    if (dim < shape_.size()) {
      offset_ += stride_[dim] * (shape_[dim] - 1);
      stride_[dim] = -stride_[dim];
      return true;
    } else {
      return false;
    }
  }

  const ShapeVector& shape() const { return shape_; }
  const StrideVector& stride() const { return stride_; }
  const std::size_t start_offset() const { return offset_; }

  // Returns the product of the shape.
  std::size_t num_elements() const { return num_elements(shape()); }

  // Returns how many elements the Layout requires for a given shape.
  static std::size_t num_elements(const ShapeVector& shape) {
    return std::accumulate(shape.begin(), shape.end(), 1,
                           std::multiplies<std::size_t>());
  }

  // Returns whether this is a contiguous layout in row-major order.
  bool IsContiguous() const {
    std::ptrdiff_t accumulator = 1;
    for (std::size_t i = 0; i < shape_.size(); ++i) {
      std::size_t ri = shape_.size() - i - 1;
      if (stride_[ri] != accumulator) {
        return false;
      }
      accumulator *= shape_[ri];
    }
    return true;
  }

  // Visit all indexes and offsets in Layout order.
  template <typename F>
  void ForEachIndexedOffset(F&& f) const {
    std::size_t num = num_elements();
    auto it = MakeIterator();
    for (std::size_t i = 0; i < num; ++i) {
      f(it.index, it.offset);
      if (i + 1 < num) Next(&it);
    }
  }

  // Returns whether the number of elements in '*this' and 'rhs' match.
  // If true call 'f' on all offsets of '*this' and 'rhs pairwise in Layout
  // order, otherwise there is no effect.
  template <typename F>
  bool PairwiseForEachOffset(const Layout& rhs, F&& f) const {
    std::size_t l_num_elements = num_elements();
    std::size_t r_num_elements = rhs.num_elements();
    if (l_num_elements != r_num_elements) {
      return false;
    }
    std::ptrdiff_t l_contig = ContiguousStride();
    std::ptrdiff_t r_contig = rhs.ContiguousStride();
    if (l_contig != 0 && r_contig != 0) {
      std::ptrdiff_t l_offset = offset_;
      std::ptrdiff_t r_offset = rhs.offset_;
      for (std::size_t i = 0; i < l_num_elements; ++i) {
        f(i * l_contig + l_offset, i * r_contig + r_offset);
      }
    } else if (l_contig != 0) {
      std::ptrdiff_t l_offset = offset_;
      auto r_it = rhs.MakeIterator();
      for (std::size_t i = 0; i < l_num_elements; ++i) {
        f(i * l_contig + l_offset, r_it.offset);
        if (i + 1 < l_num_elements) rhs.Next(&r_it);
      }
    } else if (r_contig != 0) {
      auto l_it = MakeIterator();
      std::ptrdiff_t r_offset = rhs.offset_;
      for (std::size_t i = 0; i < l_num_elements; ++i) {
        f(l_it.offset, i * r_contig + r_offset);
        if (i + 1 < l_num_elements) Next(&l_it);
      }
    } else {
      auto l_it = MakeIterator();
      auto r_it = rhs.MakeIterator();
      for (std::size_t i = 0; i < l_num_elements; ++i) {
        f(l_it.offset, r_it.offset);
        if (i + 1 < l_num_elements) {
          Next(&l_it);
          rhs.Next(&r_it);
        }
      }
    }
    return true;
  }

  // Returns whether the number of elements in '*this' and 'rhs' match and
  // calling 'p' on all offsets of '*this' and 'rhs in Layout order returns true
  // every time.
  // If a call to 'p' returns false, no further calls to 'p' are made.
  template <typename F>
  bool AllOf(const Layout& rhs, F&& f) const {
    std::size_t l_num_elements = num_elements();
    std::size_t r_num_elements = rhs.num_elements();
    if (l_num_elements != r_num_elements) {
      return false;
    }
    std::ptrdiff_t l_contig = ContiguousStride();
    std::ptrdiff_t r_contig = rhs.ContiguousStride();
    if (l_contig != 0 && r_contig != 0) {
      std::ptrdiff_t l_offset = offset_;
      std::ptrdiff_t r_offset = rhs.offset_;
      for (std::size_t i = 0; i < l_num_elements; ++i) {
        if (!f(i * l_contig + l_offset, i * r_contig + r_offset)) {
          return false;
        }
      }
    } else if (l_contig != 0) {
      std::ptrdiff_t l_offset = offset_;
      auto r_it = rhs.MakeIterator();
      for (std::size_t i = 0; i < l_num_elements; ++i) {
        if (!f(i * l_contig + l_offset, r_it.offset)) {
          return false;
        }
        if (i + 1 < l_num_elements) rhs.Next(&r_it);
      }
    } else if (r_contig != 0) {
      auto l_it = MakeIterator();
      std::ptrdiff_t r_offset = rhs.offset_;
      for (std::size_t i = 0; i < l_num_elements; ++i) {
        if (!f(l_it.offset, i * r_contig + r_offset)) {
          return false;
        }
        if (i + 1 < l_num_elements) Next(&l_it);
      }
    } else {
      auto l_it = MakeIterator();
      auto r_it = rhs.MakeIterator();
      for (std::size_t i = 0; i < l_num_elements; ++i) {
        if (!f(l_it.offset, r_it.offset)) {
          return false;
        }
        if (i + 1 < l_num_elements) {
          Next(&l_it);
          rhs.Next(&r_it);
        }
      }
    }
    return true;
  }

  // Visit all offsets in Layout order.
  template <typename F>
  void ForEachOffset(F&& f) const {
    std::size_t num = num_elements();
    std::ptrdiff_t contig = ContiguousStride();
    if (contig != 0) {
      std::ptrdiff_t offset = offset_;
      for (std::size_t i = 0; i < num; ++i) {
        f(i * contig + offset);
      }
    } else {
      auto it = MakeIterator();
      for (std::size_t i = 0; i < num; ++i) {
        f(it.offset);
        if (i + 1 < num) Next(&it);
      }
    }
  }

  // Returns whether an index is valid.
  // If it is valid offset is set to the position that index represents.
  bool GetOffset(const ShapeVector& index, std::size_t* offset) const {
    if (index.size() != stride_.size()) {
      return false;
    }
    std::size_t local_offset = offset_;
    for (std::size_t i = 0; i < stride_.size(); ++i) {
      if (index[i] >= shape_[i]) {
        return false;
      }
      local_offset += index[i] * stride_[i];
    }
    *offset = local_offset;
    return true;
  }

  // Returns whether the current Layout has a constant stride and number of
  // elements in new_shape matches num_elements(). If true the new_shape is set
  // and stride calculated. Otherwise this call has no effect.
  bool Reshape(ShapeVector new_shape) {
    std::size_t new_size = std::accumulate(new_shape.begin(), new_shape.end(),
                                           1, std::multiplies<std::size_t>());
    if (new_size != num_elements()) {
      return false;
    }
    std::ptrdiff_t stride_back = ContiguousStride();
    if (stride_back == 0) {
      return false;
    }
    stride_.clear();
    shape_ = std::move(new_shape);
    stride_.reserve(shape_.size());
    ExclusiveScan(shape_.rbegin(), shape_.rend(), std::back_inserter(stride_),
                  stride_back, std::multiplies<std::ptrdiff_t>{});
    std::reverse(stride_.begin(), stride_.end());
    return true;
  }

  // Streams out shaped data using printer for each offset.
  void PrintToStream(
      std::size_t max_num_elements, std::ostream* os,
      std::function<void(std::ostream* os, std::size_t offset)> printer) const;

 private:
  // Returns the stride if the layout has a fixed distance between all offsets.
  // If not it returns zero.
  std::ptrdiff_t ContiguousStride() const {
    if (shape_.empty()) {
      return 1;
    }
    std::ptrdiff_t accumulator = stride_.back();
    for (std::size_t i = 1; i < shape_.size(); ++i) {
      std::size_t ri = shape_.size() - i - 1;
      accumulator *= shape_[ri + 1];
      if (stride_[ri] != accumulator) {
        return 0;
      }
    }
    return stride_.back();
  }

  struct WindowIterator {
    std::size_t offset;
    std::size_t pos;
    ShapeVector index;
    const std::size_t back_idx;
  };

  // Creates an iterator for visiting all offsets.
  WindowIterator MakeIterator() const {
    return {offset_, 0, ShapeVector(shape_.size()),
            shape_.empty() ? 0 : shape_.size() - 1};
  }

  // Updates the iterator. Must not be called more than num_elements times.
  void Next(WindowIterator* iterator) const {
    ++iterator->pos;
    std::size_t back_idx = iterator->back_idx;
    ++iterator->index[back_idx];
    iterator->offset += stride_[back_idx];
    for (; back_idx != 0 && iterator->index[back_idx] == shape_[back_idx];
         --back_idx) {
      iterator->offset -= stride_[back_idx] * shape_[back_idx];
      iterator->index[back_idx] = 0;
      iterator->offset += stride_[back_idx - 1];
      ++iterator->index[back_idx - 1];
    }
  }

  // Stores the shape in row major order.
  ShapeVector shape_;

  // Stores the stride in row major order.
  StrideVector stride_;

  // Stores the start offset of this layout.
  std::size_t offset_;
};

// TensorView is for manipulating the contents of contiguous storage.
// The storage is indexed according to the Layout.
template <typename T>
class TensorView : public Layout {
 public:
  // Must be called with storage large enough to store all offsets in layout.
  // Storage can and is shared between tensor views.
  // The layouts are not however.
  TensorView(Layout layout, T* storage)
      : Layout(std::move(layout)), storage_(storage) {}

  // Iterates the tensor in Layout order. Providing the index and mutable value
  // at each location.
  template <typename F>
  void ForEachIndexedMutable(F&& f) {
    T* storage = storage_;
    ForEachIndexedOffset(
        [&f, storage](const ShapeVector& iterator, std::size_t offset) {
          f(iterator, &storage[offset]);
        });
  }

  // Iterates the tensor in Layout order, calling f with the index and value at
  // each location.
  template <typename F>
  void ForEachIndexed(F&& f) const {
    const T* storage = storage_;
    ForEachIndexedOffset(
        [&f, storage](const ShapeVector& iterator, std::size_t offset) {
          f(iterator, storage[offset]);
        });
  }

  // Iterates the tensor in Layout order, calling f with the index and mutable
  // value at each location.
  template <typename F>
  void ForEachMutable(F&& f) {
    T* storage = storage_;
    ForEachOffset([&f, storage](std::size_t offset) { f(&storage[offset]); });
  }

  // Iterates the tensor in Layout order. Providing the value at each location.
  template <typename F>
  void ForEach(F&& f) const {
    T* storage = storage_;
    ForEachOffset([&f, storage](std::size_t offset) { f(storage[offset]); });
  }

  // If '*this' matches the number of elements in 'rhs', 'op' is applied to the
  // elements of '*this' and 'rhs' and returns true, otherwise returns false.
  template <typename U, typename F>
  bool ComponentOpMutable(const TensorView<U>& rhs, F&& op) {
    T* lhs_storage = mutable_storage();
    const U* rhs_storage = rhs.storage();
    return PairwiseForEachOffset(
        rhs, [&op, lhs_storage, rhs_storage](std::size_t lhs_offset,
                                             std::size_t rhs_offset) {
          op(&lhs_storage[lhs_offset], rhs_storage[rhs_offset]);
        });
  }

  // If '*this' matches the number of elements in 'rhs', 'op' is applied to the
  // elements of '*this' and 'rhs' and returns true, otherwise returns false.
  template <typename U, typename F>
  bool ComponentOp(const TensorView<U>& rhs, F&& op) const {
    const T* lhs_storage = storage();
    const U* rhs_storage = rhs.storage();
    return PairwiseForEachOffset(
        rhs, [&op, lhs_storage, rhs_storage](std::size_t lhs_offset,
                                             std::size_t rhs_offset) {
          op(lhs_storage[lhs_offset], rhs_storage[rhs_offset]);
        });
  }

  // Returns whether *this and 'rhs' have the same shape and the elements are
  // equal.
  bool operator==(const TensorView& rhs) const {
    if (shape() != rhs.shape()) {
      return false;
    }
    const T* lhs_storage = storage();
    const T* rhs_storage = rhs.storage();
    return AllOf(rhs, [lhs_storage, rhs_storage](std::size_t lhs_offset,
                                                 std::size_t rhs_offset) {
      return lhs_storage[lhs_offset] == rhs_storage[rhs_offset];
    });
  }

  // Assigns all elements in '*this' to be the value 'rhs'.
  template <typename U>
  void Assign(U rhs) {
    ForEachMutable([rhs](T* val) { *val = rhs; });
  }

  // Multiplies all elements in '*this' by the value 'rhs'.
  template <typename U>
  void Mul(U rhs) {
    ForEachMutable([rhs](T* val) { *val *= rhs; });
  }

  // Adds to all elements in '*this' by the value 'rhs'.
  template <typename U>
  void Add(U rhs) {
    ForEachMutable([rhs](T* val) { *val += rhs; });
  }

  // Divides all elements in '*this' by the value 'rhs'.
  template <typename U>
  void Div(U rhs) {
    ForEachMutable([rhs](T* val) { *val /= rhs; });
  }

  // Subtracts all elements in '*this' by the value 'rhs'.
  template <typename U>
  void Sub(U rhs) {
    ForEachMutable([rhs](T* val) { *val -= rhs; });
  }

  // All the following member functions starting with 'C', return whether the
  // number of elements in '*this' and 'rhs' match. If not there is no effect.
  // The elements are operated on, pairwise, in their Layout order.
  //
  // Assigns '*this' component-wise with 'rhs'.
  template <typename U>
  bool CAssign(const TensorView<U>& rhs) {
    return ComponentOpMutable(rhs, [](T* v_lhs, U v_rhs) { *v_lhs = v_rhs; });
  }

  // Multiplies '*this' component-wise by 'rhs'.
  template <typename U>
  bool CMul(const TensorView<U>& rhs) {
    return ComponentOpMutable(rhs, [](T* v_lhs, U v_rhs) { *v_lhs *= v_rhs; });
  }

  // Adds '*this' component-wise by 'rhs'.
  template <typename U>
  bool CAdd(const TensorView<U>& rhs) {
    return ComponentOpMutable(rhs, [](T* v_lhs, U v_rhs) { *v_lhs += v_rhs; });
  }

  // Divides '*this' component-wise by 'rhs'.
  template <typename U>
  bool CDiv(const TensorView<U>& rhs) {
    return ComponentOpMutable(rhs, [](T* v_lhs, U v_rhs) { *v_lhs /= v_rhs; });
  }

  // Subtracts '*this' component-wise by 'rhs'.
  template <typename U>
  bool CSub(const TensorView<U>& rhs) {
    return ComponentOpMutable(rhs, [](T* v_lhs, U v_rhs) { *v_lhs -= v_rhs; });
  }

  // Compute the matrix product of 'lhs' and 'rhs' and store it in '*this'.
  template <typename U>
  bool MMul(const TensorView<U>& lhs, const TensorView<U>& rhs) {
    const auto& lhs_shape = lhs.shape();
    const auto& rhs_shape = rhs.shape();
    if (lhs_shape.size() != 2 || rhs_shape.size() != 2 ||
        lhs_shape[1] != rhs_shape[0] || shape().size() != 2 ||
        shape()[0] != lhs_shape[0] || shape()[1] != rhs_shape[1]) {
      return false;
    }
    // Map the storage used by the input tensors as Eigen matrices.
    using MatrixXU =
        Eigen::Matrix<U, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
    using MatrixXUStride = Eigen::Stride<Eigen::Dynamic, Eigen::Dynamic>;
    using MatrixXUInMap = Eigen::Map<const MatrixXU, 0, MatrixXUStride>;
    const auto& lhs_stride = lhs.stride();
    MatrixXUInMap lhs_map(lhs.storage() + lhs.start_offset(),  //
                          lhs_shape[0], lhs_shape[1],          //
                          MatrixXUStride(lhs_stride[0], lhs_stride[1]));
    const auto& rhs_stride = rhs.stride();
    MatrixXUInMap rhs_map(rhs.storage() + rhs.start_offset(),  //
                          rhs_shape[0], rhs_shape[1],          //
                          MatrixXUStride(rhs_stride[0], rhs_stride[1]));
    // Map the storage used by the output vector as an Eigen matrix.
    using MatrixXUOutMap = Eigen::Map<MatrixXU, 0, MatrixXUStride>;
    const auto& prod_stride = stride();
    MatrixXUOutMap prod_map(storage_ + start_offset(),   //
                            lhs_shape[0], rhs_shape[1],  //
                            MatrixXUStride(prod_stride[0], prod_stride[1]));
    if (lhs.storage() == storage_ || rhs.storage() == storage_) {
      // If there is potential aliasing between any of the operands and the
      // output, use eval() to compute the product on temporary storage before
      // writing it to the output tensor.
      prod_map = (lhs_map * rhs_map).eval();
    } else {
      // No aliasing is possible, compute the product directly on to the output
      // tensor.
      prod_map = lhs_map * rhs_map;
    }
    return true;
  }

  // Assigns '*this' component-wise to 'floor(rhs)'.
  void Floor() {
    ForEachMutable([](T* val) { *val = std::floor(*val); });
  }

  // Assigns '*this' component-wise to 'ceil(rhs)'.
  void Ceil() {
    ForEachMutable([](T* val) { *val = std::ceil(*val); });
  }

  // Assigns '*this' component-wise to 'round(rhs)'.
  void Round() {
    ForEachMutable([](T* val) { *val = std::round(*val); });
  }

  // Calculates the sum of all elements using T2 type as the accumulator.
  template <typename T2 = T>
  T2 Sum() const {
    T2 acc = 0;
    ForEach([&acc](T2 val) { acc += val; });
    return acc;
  }

  // Calculates the product of all elements using T2 type as the accumulator.
  template <typename T2 = T>
  T2 Product() const {
    T2 acc = 1;
    ForEach([&acc](T2 val) { acc *= val; });
    return acc;
  }

  // Calculates the sum of all elements squared using T2 type as the
  // accumulator.
  template <typename T2 = T>
  T2 LengthSquared() const {
    T2 acc = 0;
    ForEach([&acc](T2 val) { acc += val * val; });
    return acc;
  }

  // Calculates the sum of all elements squared using '*acc' as the accumulator.
  template <typename U, typename T2 = T>
  bool DotProduct(const TensorView<U>& rhs, T2* acc) const {
    *acc = 0;
    return ComponentOp(rhs,
                       [acc](T2 v_lhs, T2 v_rhs) { *acc += v_lhs * v_rhs; });
  }

  // Shuffles a rank-1 tensor.
  // If the tensor is rank-1, it shuffles its elements using the provided random
  // bit generator and returns true, otherwise it returns false.
  template <typename R>
  bool Shuffle(R* prbg) {
    if (shape().size() == 1) {
      for (std::size_t i = 1, e = shape()[0]; i < e; ++i) {
        std::uniform_int_distribution<decltype(i)> d(0, e - i);
        std::swap(storage_[start_offset() + stride()[0] * (e - i)],
                  storage_[start_offset() + stride()[0] * d(*prbg)]);
      }
      return true;
    } else {
      return false;
    }
  }

  // All the following getters and setters return whether the index is valid
  // If the index is invalid it results in a no-op.
  //
  // Assigns 'value' in location 'index'.
  bool Set(const ShapeVector& index, T value) {
    std::size_t offset;
    if (GetOffset(index, &offset)) {
      storage_[offset] = value;
      return true;
    } else {
      return false;
    }
  }

  // Iff the index is valid, retrieve the value at 'index' into 'value'.
  // Returns whether the index is valid.
  bool Get(const ShapeVector& index, T* value) const {
    std::size_t offset;
    if (GetOffset(index, &offset)) {
      *value = storage_[offset];
      return true;
    } else {
      return false;
    }
  }

  // 1D overload of Set(const ShapeVector& index, T value).
  // Iff the index is valid, set the value at 'index' to 'value'.
  // Returns whether the index is valid.
  bool Set(std::size_t index, T value) {
    if (shape().size() == 1 && index < shape()[0]) {
      storage_[start_offset() + index * stride()[0]] = value;
      return true;
    } else {
      return false;
    }
  }

  // 1D overload of Get(const ShapeVector& index, T* value).
  // Iff the index is valid, retrieve the value at 'index' into 'value'.
  // Returns whether the index is valid.
  bool Get(std::size_t index, T* value) const {
    if (shape().size() == 1 && index < shape()[0]) {
      *value = storage_[start_offset() + index * stride()[0]];
      return true;
    } else {
      return false;
    }
  }

  // Returns the index of max value in Tensor.
  std::optional<ShapeVector> ArgMaxElement() const {
    if (auto result = FlatReduceTopOne(std::greater<T>{}); result.has_value()) {
      return UnravelIndex(shape(), result->first);
    } else {
      return std::nullopt;
    }
  }

  // Returns the index of min value in Tensor.
  std::optional<ShapeVector> ArgMinElement() const {
    if (auto result = FlatReduceTopOne(std::less<T>{}); result.has_value()) {
      return UnravelIndex(shape(), result->first);
    } else {
      return std::nullopt;
    }
  }

  // Returns the max value in of the Tensor.
  std::optional<T> MaxElement() const {
    if (auto result = FlatReduceTopOne(std::greater<T>{}); result.has_value()) {
      return result->second;
    } else {
      return std::nullopt;
    }
  }

  // Returns the min value in of the Tensor.
  std::optional<T> MinElement() const {
    if (auto result = FlatReduceTopOne(std::less<T>{}); result.has_value()) {
      return result->second;
    } else {
      return std::nullopt;
    }
  }

  // Transform reduce `rhs_view` across given `dim` with the operation index of
  // max element storing the result in *this. Returns whether the operation
  // shapes are correct and reduce operation was successful.
  template <typename U>
  bool ArgMax(const TensorView<U>& rhs_view, int dim) {
    return ReduceTopOneIndex(rhs_view, dim, std::greater<T>{});
  }

  // Transform reduce `rhs_view` across given `dim` with the operation: index
  // of min element storing the result in *this. Returns whether the operation
  // shapes are correct and reduce operation was successful.
  template <typename U>
  bool ArgMin(const TensorView<U>& rhs_view, int dim) {
    return ReduceTopOneIndex(rhs_view, dim, std::less<T>{});
  }

  // Transform reduce `rhs_view` across given `dim` with the operation: max
  // element storing the result in *this. Returns whether the operation shapes
  // are correct and reduce operation was successful.
  template <typename U>
  bool Max(const TensorView<U>& rhs_view, int dim) {
    return ReduceTopOne(rhs_view, dim, std::greater<T>{});
  }

  // Transform reduce `rhs_view` across given `dim` with the operation: max
  // element storing the result in *this. Returns whether the operation shapes
  // are correct and reduce operation was successful.
  template <typename U>
  bool Min(const TensorView<U>& rhs_view, int dim) {
    return ReduceTopOne(rhs_view, dim, std::less<T>{});
  }

  // Enable streaming of Tensor View.
  friend std::ostream& operator<<(std::ostream& os, const TensorView& view) {
    view.PrintToStream(/*max_num_elements=*/1024, &os);
    return os;
  }

  void PrintToStream(std::size_t max_num_elements, std::ostream* os) const {
    const T* data = storage();
    Layout::PrintToStream(max_num_elements, os,
                          [data](std::ostream* os, std::size_t offset) {
                            // The '+' expression promotes the value to make
                            // char's print as integers.
                            (*os) << +data[offset];
                          });
  }

  const T* storage() const { return storage_; }
  T* mutable_storage() { return storage_; }

 private:
  // Reduces `rhs` with `*this` across axis `dim`. Each row in dimension `dim`
  // is reduced with the 3 provided callbacks. An accumulator is initialised
  // with the return value of `init` and the first value of the row. The
  // accumulator is then reduced with callback `reduce` and each subsequent
  // value in the row. The accumulated value is then converted for storage on
  // the corresponding value in `this` with the `finalise` callback.
  //
  // Signatures of callbacks:
  //   Accumulator init(std::size_t first_index, U value);
  //   Accumulator reduce(std::size_t index, Accumulator accumulator, U value);
  //   T finalise(std::size_t num_elements, Accumulator acumulator);
  //
  // Example:
  //   `this` shape (3, 5), `rhs` shape (3, 4, 5) and `dim` = 1:
  //   for all i, j in [0, 3), [0, 5) {
  //     auto accumulator = init(0, rhs(i, 0, j))
  //     accumulator = reduce(1, accumulator, rhs(i, 1, j))
  //     accumulator = reduce(2, accumulator, rhs(i, 2, j))
  //     accumulator = reduce(3, accumulator, rhs(i, 3, j))
  //     this->Set(i, j, finalise(4, accumulator))
  //   }
  template <typename U, typename InitOp, typename ReduceOp,
            typename TransformOp>
  bool ReducePairwise(const TensorView<U>& rhs, int dim, InitOp&& init,
                      ReduceOp&& reduce, TransformOp&& finalise) {
    Layout rhs_reduced = rhs;
    if (!rhs_reduced.Select(dim, 0)) {
      return false;
    }
    T* lhs_storage = mutable_storage();
    const U* rhs_storage = rhs.storage();
    std::size_t rhs_shape = rhs.shape()[dim];
    std::ptrdiff_t rhs_stride = rhs.stride()[dim];
    return PairwiseForEachOffset(
        rhs_reduced,
        [&init, &reduce, &finalise, lhs_storage, rhs_storage, rhs_stride,
         rhs_shape](std::size_t lhs_offset, std::size_t rhs_offset) {
          auto accumulator = init(0, rhs_storage[rhs_offset]);
          for (std::size_t i = 1; i < rhs_shape; ++i) {
            accumulator = reduce(i, accumulator,
                                 rhs_storage[rhs_offset + i * rhs_stride]);
          }
          lhs_storage[lhs_offset] = finalise(rhs_shape, accumulator);
        });
  }

  // Sets top-one value in `this` in each row across axis `dim` in `rhs`.
  template <typename U, typename Compare>
  bool ReduceTopOne(const TensorView<U>& rhs, int dim, Compare&& compare) {
    return ReducePairwise(
        rhs, dim,
        /*init=*/[](std::size_t, U value) { return static_cast<T>(value); },
        /*reduce=*/
        [&compare](std::size_t, T accumulator, U value) {
          return compare(value, accumulator) ? static_cast<T>(value)
                                             : accumulator;
        },
        /*finalise=*/[](std::size_t, T accumulator) { return accumulator; });
  }

  // Sets top-one index in `this` in each row across axis `dim` in `rhs`.
  template <typename U, typename Compare>
  bool ReduceTopOneIndex(const TensorView<U>& rhs, int dim, Compare&& compare) {
    using Accumulator = std::pair<std::size_t, U>;
    return ReducePairwise(
        rhs, dim,
        /*init=*/
        [](std::size_t first_index, U value) {
          return Accumulator(first_index, value);
        },
        /*reduce=*/
        [&compare](std::size_t index, Accumulator accumulator, T value) {
          if (compare(value, accumulator.second)) {
            accumulator.first = index;
            accumulator.second = value;
          }
          return accumulator;
        },
        /*finalise=*/
        [](std::size_t, Accumulator accumulator) { return accumulator.first; });
  }

  // Returns flattened index and value of top-one value in `this`.
  template <typename Compare>
  std::optional<std::pair<std::size_t, T>> FlatReduceTopOne(
      Compare&& compare) const {
    std::optional<std::pair<std::size_t, T>> top;
    std::size_t index = 0;
    ForEach([&compare, &top, &index](T value) {
      if (!top.has_value() || compare(value, (*top).second)) {
        top = std::make_pair(index, value);
      }
      ++index;
    });
    return top;
  }

  // Contiguous storage where the values are kept.
  T* storage_;
};

}  // namespace tensor
}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_TENSOR_TENSOR_VIEW_H_
