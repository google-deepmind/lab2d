// Copyright (C) 2019 The DMLab2D Authors.
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

#ifndef DMLAB2D_SYSTEM_MATH_MATH2D_H_
#define DMLAB2D_SYSTEM_MATH_MATH2D_H_

#include <cstdlib>
#include <ostream>

namespace deepmind::lab2d::math {

// Absolute position in 2D.
// Origin assumed to be top left.
// -------> x
// |
// |
// v
// y
struct Position2d;

// Relative offset of two positions in 2D.
struct Vector2d;

// The `width` and `height` of a rectangle.
struct Size2d;

// Absolute position and orientation of an object in 2D.
struct Transform2d;

// Orientation of an object.
//       N
//       ^
//       |
// W <---+---> E
//       |
//       v
//       S
enum class Orientation2d {
  kNorth,
  kEast,
  kSouth,
  kWest,
};

// Rotation between two directions.
// Any two Orientation2s differ by a Rotation2d, and Rotation2s can be composed.
//
// For example, these are the differences in orientation to kNorth:
//
//               kNorth + k0
//                    ^
//                    |
// kNorth + k270  <---+--->  kNorth + k90
//                    |
//                    v
//              kNorth + k180
//
enum class Rotate2d {
  k0,
  k90,
  k180,
  k270,
};

constexpr inline Orientation2d operator+(Orientation2d lhs, Rotate2d rhs) {
  return static_cast<Orientation2d>(
      (static_cast<unsigned int>(lhs) + static_cast<unsigned int>(rhs)) % 4);
}

constexpr inline Orientation2d operator-(Orientation2d lhs, Rotate2d rhs) {
  return static_cast<Orientation2d>(
      (static_cast<unsigned int>(lhs) + 4 - static_cast<unsigned int>(rhs)) %
      4);
}

constexpr inline Orientation2d operator+(Rotate2d lhs, Orientation2d rhs) {
  return static_cast<Orientation2d>(
      (static_cast<unsigned int>(rhs) + static_cast<unsigned int>(lhs)) % 4);
}

constexpr inline Rotate2d operator-(Orientation2d lhs, Orientation2d rhs) {
  return static_cast<Rotate2d>(
      (static_cast<unsigned int>(lhs) + 4 - static_cast<unsigned int>(rhs)) %
      4);
}

constexpr inline Rotate2d operator+(Rotate2d lhs, Rotate2d rhs) {
  return static_cast<Rotate2d>(
      (static_cast<unsigned int>(lhs) + static_cast<unsigned int>(rhs)) % 4);
}

constexpr inline Rotate2d operator-(Rotate2d lhs, Rotate2d rhs) {
  return static_cast<Rotate2d>(
      (static_cast<unsigned int>(lhs) + 4 - static_cast<unsigned int>(rhs)) %
      4);
}

// Returns the orientation of `object` as seen from `viewer`.
constexpr inline Orientation2d FromView(Orientation2d viewer,
                                        Orientation2d object) {
  return object + (Orientation2d::kNorth - viewer);
}

struct Position2d {
  int x;
  int y;
  constexpr Position2d& operator+=(Vector2d rhs);
  constexpr Position2d& operator-=(Vector2d rhs);
  friend constexpr Position2d operator+(Position2d lhs, Vector2d rhs);
  friend constexpr Position2d operator+(Vector2d lhs, Position2d rhs);
  friend constexpr Position2d operator-(Position2d lhs, Vector2d rhs);
  friend constexpr Vector2d operator-(Position2d lhs, Position2d rhs);
  friend constexpr bool operator==(Position2d lhs, Position2d rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }
  friend constexpr bool operator!=(Position2d lhs, Position2d rhs) {
    return lhs.x != rhs.x || lhs.y != rhs.y;
  }
};

struct Vector2d {
  int x;
  int y;
  constexpr Vector2d& operator+=(Vector2d rhs);
  constexpr Vector2d& operator-=(Vector2d rhs);
  constexpr Vector2d& operator*=(int);
  constexpr Vector2d& operator/=(int);
  constexpr Vector2d& operator*=(Rotate2d);

  constexpr Vector2d operator-() { return {-x, -y}; }

  friend constexpr Vector2d operator*(Vector2d lhs, int rhs);
  friend constexpr Vector2d operator*(int lhs, Vector2d rhs);
  friend constexpr Vector2d operator/(Vector2d lhs, int rhs);
  friend constexpr Vector2d operator+(Vector2d lhs, Vector2d rhs);
  friend constexpr Vector2d operator-(Vector2d lhs, Vector2d rhs);

  friend constexpr Vector2d operator*(Vector2d lhs, Rotate2d rhs);
  friend constexpr Vector2d operator*(Rotate2d lhs, Vector2d rhs);

  friend constexpr bool operator==(Vector2d lhs, Vector2d rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }
  constexpr static Vector2d North() { return {0, -1}; }
  constexpr static Vector2d East() { return {1, 0}; }
  constexpr static Vector2d South() { return {0, 1}; }
  constexpr static Vector2d West() { return {-1, 0}; }
  constexpr static Vector2d Zero() { return {0, 0}; }
  constexpr static Vector2d FromOrientation(Orientation2d orientation) {
    switch (orientation) {
      case Orientation2d::kNorth:
        return North();
      case Orientation2d::kEast:
        return East();
      case Orientation2d::kSouth:
        return South();
      case Orientation2d::kWest:
        return West();
    }
    std::abort();
  }
};

struct Size2d {
  constexpr int Area() const { return width * height; }
  constexpr bool Contains(Position2d pos) const {
    return 0 <= pos.x && pos.x < width && 0 <= pos.y && pos.y < height;
  }
  int width;
  int height;
  friend constexpr bool operator==(Size2d lhs, Size2d rhs) {
    return lhs.width == rhs.width && lhs.height == rhs.height;
  }
};

struct Transform2d {
  friend constexpr bool operator==(Transform2d lhs, Transform2d rhs) {
    return lhs.position == rhs.position && lhs.orientation == rhs.orientation;
  }

  // Given a transform ({10, 3}, E):
  //   +-------------> E
  //   |   X-->u
  //   |
  //   |  S<-----+
  //   |         |
  //   |         |
  //   |         v
  //   v         E
  //   S
  //
  //   `X` position is (4, 1) in absolute-space.
  //   `X` position is (6, -2) in relative-space.
  //   `u` vector is (3, 0) in absolute-space.
  //   `u` vector is (0, -3) in absolute-space.

  // Returns absolute-space orientation of a relative-space orientation.
  constexpr Orientation2d ToAbsoluteSpace(Orientation2d orientation_ws) const {
    return orientation_ws + (orientation - Orientation2d::kNorth);
  }

  // Returns absolute-space vector of a relative-space vector.
  constexpr Vector2d ToAbsoluteSpace(const Vector2d& direction_rs) const {
    return (orientation - Orientation2d::kNorth) * direction_rs;
  }

  // Returns absolute-space position of a relative-space position.
  constexpr Position2d ToAbsoluteSpace(const Position2d& position_rs) const {
    return position + ToAbsoluteSpace(position_rs - Position2d{0, 0});
  }

  // Returns relative-space orientation of a absolute-space orientation.
  constexpr Vector2d ToRelativeSpace(const Vector2d& direction_ws) const {
    return (Orientation2d::kNorth - this->orientation) * direction_ws;
  }

  // Returns relative-space vector of a absolute-space vector.
  constexpr Position2d ToRelativeSpace(const Position2d& position_ws) const {
    return ToRelativeSpace(position_ws - position) + Position2d{0, 0};
  }

  // Returns relative-space position of a absolute-space position.
  constexpr Orientation2d ToRelativeSpace(Orientation2d orientation_ws) const {
    return orientation_ws + (Orientation2d::kNorth - orientation);
  }

  Position2d position;
  Orientation2d orientation;
};

inline constexpr Position2d& Position2d::operator+=(Vector2d rhs) {
  x += rhs.x;
  y += rhs.y;
  return *this;
}

inline constexpr Position2d& Position2d::operator-=(Vector2d rhs) {
  x -= rhs.x;
  y -= rhs.y;
  return *this;
}

inline constexpr Position2d operator+(Position2d lhs, Vector2d rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y};
}
inline constexpr Position2d operator+(Vector2d lhs, Position2d rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y};
}
inline constexpr Position2d operator-(Position2d lhs, Vector2d rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y};
}
inline constexpr Vector2d operator-(Position2d lhs, Position2d rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y};
}

inline constexpr Vector2d& Vector2d::operator+=(Vector2d rhs) {
  x += rhs.x;
  y += rhs.y;
  return *this;
}
inline constexpr Vector2d& Vector2d::operator-=(Vector2d rhs) {
  x -= rhs.x;
  y -= rhs.y;
  return *this;
}
inline constexpr Vector2d& Vector2d::operator*=(int rhs) {
  x *= rhs;
  y *= rhs;
  return *this;
}
inline constexpr Vector2d& Vector2d::operator/=(int rhs) {
  x /= rhs;
  y /= rhs;
  return *this;
}

inline constexpr Vector2d operator*(Vector2d lhs, int rhs) {
  return {lhs.x * rhs, lhs.y * rhs};
}

inline constexpr Vector2d operator*(int lhs, Vector2d rhs) {
  return {lhs * rhs.x, lhs * rhs.y};
}

inline constexpr Vector2d operator/(Vector2d lhs, int rhs) {
  return {lhs.x / rhs, lhs.y / rhs};
}

inline constexpr Vector2d operator+(Vector2d lhs, Vector2d rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y};
}

inline constexpr Vector2d operator-(Vector2d lhs, Vector2d rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y};
}

inline constexpr Vector2d& Vector2d::operator*=(Rotate2d rhs) {
  switch (rhs) {
    case Rotate2d::k0:
      return *this;
    case Rotate2d::k90:
      *this = {-y, x};
      return *this;
    case Rotate2d::k180:
      *this = {-x, -y};
      return *this;
    case Rotate2d::k270:
      *this = {y, -x};
      return *this;
  }
  std::abort();
}

inline constexpr Vector2d operator*(Vector2d lhs, Rotate2d rhs) {
  Vector2d result = lhs;
  result *= rhs;
  return result;
}

inline constexpr Vector2d operator*(Rotate2d lhs, Vector2d rhs) {
  Vector2d result = rhs;
  result *= lhs;
  return result;
}

inline std::ostream& operator<<(std::ostream& out, const Vector2d& val) {
  return out << "V(" << val.x << ", " << val.y << ")";
}

inline std::ostream& operator<<(std::ostream& out, const Position2d& val) {
  return out << "P(" << val.x << ", " << val.y << ")";
}

inline std::ostream& operator<<(std::ostream& out, const Rotate2d& val) {
  return out << "R" << static_cast<int>(val) * 90 << ")";
}

inline std::ostream& operator<<(std::ostream& out, const Size2d& val) {
  return out << "S(w: " << val.width << ", h: " << val.height << ")";
}

inline std::ostream& operator<<(std::ostream& out, const Orientation2d& val) {
  switch (val) {
    case Orientation2d::kNorth:
      return out << "O(N)";
    case Orientation2d::kEast:
      return out << "O(E)";
    case Orientation2d::kSouth:
      return out << "O(S)";
    case Orientation2d::kWest:
      return out << "O(W)";
  }
  return out << "<Invalid Orientation2d>";
}

inline std::ostream& operator<<(std::ostream& out, const Transform2d& val) {
  return out << "T(p: " << val.position << ", o: " << val.orientation << ")";
}

}  // namespace deepmind::lab2d::math

#endif  // DMLAB2D_SYSTEM_MATH_MATH2D_H_
