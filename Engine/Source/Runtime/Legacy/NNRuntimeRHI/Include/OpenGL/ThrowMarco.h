/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#pragma once
#include "Exception.h"
#include <NNCore/Interface/HAssert.h>

#ifdef _DEBUG
#define GL_CLEAR_ERROR() NN::Runtime::OpenGL::Exception::ClearError()
#define GL_CHECK_ERROR() NN::Runtime::OpenGL::Exception::CheckError()
#define GL_THROW_INFO(call) \
NN::Runtime::OpenGL::Exception::ClearError(); \
call;  \
H_ASSERT(!NN::Runtime::OpenGL::Exception::CheckError(#call,__FILE__,__LINE__),#call)

#else
#define OPENGL_CLEAR_ERROR() 
#define OPENGL_CHECK_ERROR() 
#define GL_THROW_INFO(x) x
#endif 