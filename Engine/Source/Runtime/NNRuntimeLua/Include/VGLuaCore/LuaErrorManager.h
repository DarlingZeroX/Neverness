#ifndef LUA_EROOR_MANAGER_H
#define LUA_EROOR_MANAGER_H

#include "../Lua/luaconf.h"

#ifdef __cplusplus
extern "C" {
#endif
	void VGLuaCoreSetErrorLineNumber(int line);

	LUA_API int VGLuaCoreGetErrorLineNumber();
#ifdef __cplusplus
}
#endif

#endif