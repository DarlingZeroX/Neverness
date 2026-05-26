/*
 * This source file is part of Neverness Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 * See the LICENSE file in the project root for details.
 */

/// stb_image 实现编译单元
/// 此文件只编译一次，定义 STB_IMAGE_IMPLEMENTATION

// 实现单元：定义 STB_IMAGE_IMPLEMENTATION 一次
// 不使用 STB_IMAGE_STATIC，使函数具有外部链接供 NNImageCodec.cpp 调用
#define STB_IMAGE_IMPLEMENTATION
#include <NNCore/Include/Stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <NNCore/Include/Stb/stb_image_write.h>
