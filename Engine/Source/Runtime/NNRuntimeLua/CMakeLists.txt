# 子目录首行：强制初始化C环境
enable_language(C CXX)

# 设置C标准（Lua源码需ANSI C）
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

# 开启utf-8编码支持
if (MSVC)
    add_compile_options(/utf-8)
endif()
 
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeLuaSources.cmake)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_definitions(-DLUA_BUILD_AS_DLL)

add_library(NevernessRuntime-Lua SHARED)
target_sources(NevernessRuntime-Lua PRIVATE ${NNRuntimeLua_SOURCES})

target_include_directories(NevernessRuntime-Lua PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-Lua PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-Lua PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include/Lua")
target_include_directories(NevernessRuntime-Lua PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")

target_link_libraries(NevernessRuntime-Lua PRIVATE NevernessCore-Core)

set_target_properties(NevernessRuntime-Lua PROPERTIES CXX_STANDARD 17)
