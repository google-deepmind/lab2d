// Copyright (C) 2017-2019 The DMLab2D Authors.
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

#ifndef DMLAB2D_LIB_UTIL_VISIT_SET_DIFFERENCE_AND_INTERSECTION_H_
#define DMLAB2D_LIB_UTIL_VISIT_SET_DIFFERENCE_AND_INTERSECTION_H_

namespace deepmind::lab2d {

// Let A and B be the sets given respectively by the ranges [first1, last1) and
// [first2, last2). The ranges shall be ordered and unique with respect to "<".
//
// VisitSetDifferencesAndIntersection calls `f1` for every element in A \ B,
// `f2` for every element in B \ A, and `f_both` for every element in A ∩ B;
// elements are visited by by f1, f2, f_both in their given order. Two elements
// a ∈ A and b ∈ B are considered to lie in the intersection A ∩ B if both a < b
// and b < a are false.
//
// If the value types of `InputIt1` and `InputIt2` are not the same, the
// behaviour of this function may be surprising.
//
template <typename InputIt1, typename InputIt1Sen,  //
          typename InputIt2, typename InputIt2Sen,  //
          typename Func1, typename Func2, typename FuncBoth>
void VisitSetDifferencesAndIntersection(  //
    InputIt1 first1, InputIt1Sen last1,   //
    InputIt2 first2, InputIt2Sen last2,   //
    Func1 f1, Func2 f2, FuncBoth f_both) {
  while (first1 != last1 && first2 != last2) {
    if (*first1 < *first2) {
      f1(*first1);
      ++first1;
    } else if (*first2 < *first1) {
      f2(*first2);
      ++first2;
    } else {
      f_both(*first1);
      ++first1;
      ++first2;
    }
  }

  // Visit the remaining elements.
  for (; first1 != last1; ++first1) f1(*first1);
  for (; first2 != last2; ++first2) f2(*first2);
}

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_UTIL_VISIT_SET_DIFFERENCE_AND_INTERSECTION_H_
