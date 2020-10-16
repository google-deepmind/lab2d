// Copyright (C) 2017-2020 The DMLab2D Authors.
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

#include "dmlab2d/system/file_system/lua/file_system.h"

#include <cstddef>

#include "absl/memory/memory.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lua/class.h"
#include "dmlab2d/lua/push.h"
#include "dmlab2d/lua/read.h"
#include "dmlab2d/system/file_system/file_system.h"
#include "dmlab2d/util/file_reader.h"
#include "dmlab2d/util/file_reader_types.h"
#include "dmlab2d/util/files.h"

namespace deepmind::lab2d {
namespace {

class LuaFileSystem : public lua::Class<LuaFileSystem> {
  friend class Class;
  static const char* ClassName() { return "system.FilesystemModule"; }

 public:
  // '*ctx' owned by the caller and should out-live this object.
  explicit LuaFileSystem(const FileSystem* ctx) : ctx_(ctx) {}

  static void Register(lua_State* L) {
    const Class::Reg methods[] = {
        {"runFiles", Member<&LuaFileSystem::Runfiles>},
        {"loadFileToString", Member<&LuaFileSystem::LoadFileToString>},
    };
    Class::Register(L, methods);
  }

 private:
  lua::NResultsOr Runfiles(lua_State* L) {
    lua::Push(L, ctx_->Runfiles());
    return 1;
  }

  lua::NResultsOr LoadFileToString(lua_State* L) {
    std::string file_name;
    if (!lua::Read(L, -1, &file_name)) {
      return "Must supply file name.";
    }
    util::FileReader file(ctx_->ReadOnlyFileSystem(), file_name.c_str());
    if (!file.Success()) {
      return file.Error();
    }
    std::size_t size;
    if (!file.GetSize(&size)) {
      return file.Error();
    }

    auto buffer = absl::make_unique<char[]>(size);
    if (!file.Read(0, size, buffer.get())) {
      return file.Error();
    }
    lua_pushlstring(L, buffer.get(), size);
    return 1;
  }

  const FileSystem* ctx_;
};

}  // namespace

lua::NResultsOr LuaFileSystemRequire(lua_State* L) {
  if (const auto* ctx = static_cast<const FileSystem*>(
          lua_touserdata(L, lua_upvalueindex(1)))) {
    LuaFileSystem::Register(L);
    LuaFileSystem::CreateObject(L, ctx);
    return 1;
  } else {
    return "Missing FileSystem context!";
  }
}

}  // namespace deepmind::lab2d
