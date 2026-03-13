// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// For the latest information, see https://github.com/DarlingZeroX/VisionGal
// See the LICENSE file in the project root for details.

#include <gtest/gtest.h>
#include "Include/TestProjectCreator.hpp"
#include "Include/TestVGLauncherData.hpp"

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);

	EXPECT_EQ(1 + 1, 2);
	EXPECT_EQ(1 + 1, 2);

	return RUN_ALL_TESTS();
}