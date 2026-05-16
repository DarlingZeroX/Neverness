// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include <SDL3/SDL_assert.h>
//#include "../System/HMessageBox.h"

#ifdef _DEBUG

//#define H_ASSERT(v,msg) if(!(v)) { NN::Core::HMessageBox::ShowSimple(0, "Assert", msg); SDL_TriggerBreakpoint();}
//#define H_ASSERT_W(v,msg) if(!(v)) {NN::Core::HMessageBox::ShowSimple(0, "Assert", msg); SDL_TriggerBreakpoint();}
//#define H_ASSERT_ALWAYS_W(msg)  { NN::Core::HMessageBox::ShowSimple(0, "Assert", msg);SDL_TriggerBreakpoint();}
//#define H_ASSERT_ALWAYS(msg)  { NN::Core::HMessageBox::ShowSimple(0, "Assert", msg);SDL_TriggerBreakpoint();}

#define H_ASSERT(v,msg) if(!(v)) {  SDL_TriggerBreakpoint();}
#define H_ASSERT_W(v,msg) if(!(v)) { SDL_TriggerBreakpoint();}
#define H_ASSERT_ALWAYS_W(msg)  { SDL_TriggerBreakpoint();}
#define H_ASSERT_ALWAYS(msg)  { SDL_TriggerBreakpoint();}

#define H_ASSERT_EQ(a,b) if((a) != (b)) SDL_TriggerBreakpoint();
#define H_ASSERT_NULL(x) if((x) != NULL) SDL_TriggerBreakpoint();
#define H_ASSERT_TRUE(x) H_ASSERT(x, "ASSERT TRUE")
#define H_ASSERT_FALSE(x) if(x) SDL_TriggerBreakpoint();
#define H_ASSERT_NOT_NULL(x) if((x) == NULL) SDL_TriggerBreakpoint();

#else

#define H_ASSERT(v,msg)
#define H_ASSERT_W(v,msg)
#define H_ASSERT_ALWAYS_W(msg)
#define H_ASSERT_ALWAYS(msg)

#define H_ASSERT_EQ(a,b) 
#define H_ASSERT_NULL(x) 
#define H_ASSERT_TRUE(x) 
#define H_ASSERT_FALSE(x) 
#define H_ASSERT_NOT_NULL(x) 

#endif 
