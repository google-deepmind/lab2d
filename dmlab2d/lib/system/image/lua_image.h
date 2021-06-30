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

#ifndef DMLAB2D_LIB_SYSTEM_IMAGE_LUA_IMAGE_H_
#define DMLAB2D_LIB_SYSTEM_IMAGE_LUA_IMAGE_H_

#include "dmlab2d/lib/lua/lua.h"

namespace deepmind::lab2d {

// Returns a table of image related functions:
// * load(path): [-1, +1, e]
//   Loads supported PNG format with the suffix '.png' and returns a byte tensor
//   with a shape {H, W, ChannelCount}. ChannelCount depends on the type of the
//   PNG being loaded.
//   Supports only non-paletted images with 8 bits per channel.
// * scale(src, tgt_height, tgt_width): [-3, +1, e]
//   Scales the image contained in byte tensor 'src' to shape {tgt_height,
//   tgt_width, ChannelCount}, where ChannelCount is the number of channels used
//   by 'src'. Returns the scaled image.
//   Supports only contiguous tensors as input.
// Must be called with Lua upvalue pointing to a DeepMindReadOnlyFileSystem.

// [0, +1, -]
int LuaImageRequire(lua_State* L);

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_IMAGE_LUA_IMAGE_H_
