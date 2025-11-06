#pragma once

#include "Exception.h"
#include <HCore/Include/Core/HAssert.h>

#ifdef _DEBUG
#define GL_CLEAR_ERROR() VISIONGAL_OPENGL_NAMESPACE::Exception::ClearError()
#define GL_CHECK_ERROR() VISIONGAL_OPENGL_NAMESPACE::Exception::CheckError()
#define GL_THROW_INFO(call) \
VISIONGAL_OPENGL_NAMESPACE::Exception::ClearError(); \
call;  \
H_ASSERT(!VISIONGAL_OPENGL_NAMESPACE::Exception::CheckError(#call,__FILE__,__LINE__),#call)

#else
#define OPENGL_CLEAR_ERROR() 
#define OPENGL_CHECK_ERROR() 
#define GL_THROW_INFO(x) x
#endif 