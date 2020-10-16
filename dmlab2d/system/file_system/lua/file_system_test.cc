// Copyright (C) 2019-2020 The DMLab2D Authors.
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

#include <cstring>
#include <random>

#include "absl/strings/string_view.h"
#include "dmlab2d/lua/bind.h"
#include "dmlab2d/lua/call.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or_test_util.h"
#include "dmlab2d/lua/push_script.h"
#include "dmlab2d/lua/read.h"
#include "dmlab2d/lua/vm.h"
#include "dmlab2d/lua/vm_test_util.h"
#include "dmlab2d/system/file_system/file_system.h"
#include "dmlab2d/util/default_read_only_file_system.h"
#include "dmlab2d/util/file_reader_types.h"
#include "dmlab2d/util/files.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::testing::Eq;

class FileSystemTest : public lua::testing::TestWithVm {
 protected:
  FileSystemTest()
      : fs_(util::GetTempDirectory() + "/dmlab2d_filesystem_test",
            util::DefaultReadOnlyFileSystem()) {
    vm()->AddCModuleToSearchers("system.file_system",
                                &lua::Bind<LuaFileSystemRequire>, {&fs_});
    util::MakeDirectory(fs_.Runfiles());
  }

  ~FileSystemTest() override { util::RemoveDirectory(fs_.Runfiles()); }

  FileSystem fs_;
};

constexpr char kFileSystem[] = R"(
local file_system = require 'system.file_system'
local runFiles = file_system:runFiles()
return file_system:loadFileToString(runFiles .. '/content')
)";

TEST_F(FileSystemTest, ReadStringEvents) {
  absl::string_view content = "Test String";
  util::SetContents(fs_.Runfiles() + "/content", content);
  ASSERT_THAT(lua::PushScript(L, kFileSystem, "kFileSystem"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  absl::string_view result;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &result)));
  EXPECT_THAT(result, Eq(content));
}

}  // namespace
}  // namespace deepmind::lab2d
