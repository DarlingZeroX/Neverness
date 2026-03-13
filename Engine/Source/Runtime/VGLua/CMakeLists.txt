# 子目录首行：强制初始化C环境
enable_language(C CXX)

# 设置C标准（Lua源码需ANSI C）
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

# 为 MSVC 编译器添加 /Zc:__cplusplus 选项
#if (MSVC)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
#endif()

# 开启utf-8编码支持
if (MSVC)
    add_compile_options(/utf-8)
endif()
 
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# 将源代码添加到此项目的可执行文件。
file(GLOB SRC_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lapi.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lauxlib.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lbaselib.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lcode.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lcorolib.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lctype.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/ldblib.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/ldebug.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/ldo.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/ldump.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lfunc.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lgc.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/linit.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/liolib.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/llex.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lmathlib.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lmem.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/loadlib.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lobject.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lopcodes.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/loslib.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lparser.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lstate.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lstring.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lstrlib.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/ltable.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/ltablib.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/ltm.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lundump.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lutf8lib.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lvm.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/Lua/lzio.c"

	"${CMAKE_CURRENT_SOURCE_DIR}/Source/VGLuaCore/LuaLocalizator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Source/VGLuaCore/LuaErrorManager.cpp"

	#"${PROJECT_SOURCE_DIR}/VGLua/Source/lua.c"
	#"${PROJECT_SOURCE_DIR}/VGLua/Source/luac.c"
	#"${PROJECT_SOURCE_DIR}/VGLua/Source/*.c"
)

# 收集头文件（假设头文件在 Include 目录下）
file(GLOB HEADER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"  # 递归匹配所有 .h 文件
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/*.hpp"  # 递归匹配所有 .hpp 文件
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/VGLuaCore/*.h"  # 递归匹配所有 .hpp 文件
)

# 缺失关键配置 ↓
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)  # 强制导出所有符号[1,3,4](@ref)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")  # 确保.lib生成在lib目录

# 定义动态库编译宏（关键！）
add_definitions(-DLUA_BUILD_AS_DLL)

# 创建动态库并添加源文件和头文件（头文件仅用于 IDE 显示）
add_library(VGLua SHARED
    ${SRC_FILES}
    ${HEADER_FILES}  # 重要：将头文件添加到目标中
)

target_include_directories(VGLua PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(VGLua PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include/Lua")
target_include_directories(VGLua PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")

target_link_libraries(VGLua PRIVATE HCore)

#set_target_properties(VGLua PROPERTIES LINKER_LANGUAGE C)
set_target_properties(VGLua PROPERTIES CXX_STANDARD 17)



