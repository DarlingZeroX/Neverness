#include "VGLuaCore/LuaErrorManager.h"

static int s_VGLuaCoreSetErrorLineNumber = 0;

void VGLuaCoreSetErrorLineNumber(int line)
{
	s_VGLuaCoreSetErrorLineNumber = line;
}

int VGLuaCoreGetErrorLineNumber()
{
	return s_VGLuaCoreSetErrorLineNumber;
}