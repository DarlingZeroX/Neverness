# NNNativeEngineAPI — Native Engine Service ABI（僅頭文件契約層，無 Stub 實作）。

# 子目录首行：强制初始化C环境
#enable_language(C CXX)
#
## 设置C标准（Lua源码需ANSI C）
#set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_STANDARD_REQUIRED ON)
## 👇 这一行是关键！强制 MSVC 启用 C11 标准模式
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP /std:c11")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 开启utf-8编码支持
if (MSVC)
    add_compile_options(/utf-8)
endif()

add_library(NevernessRuntime-NativeEngineAPI INTERFACE)

target_include_directories(NevernessRuntime-NativeEngineAPI
	INTERFACE
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
)

add_library(NNNativeEngineAPI ALIAS NevernessRuntime-NativeEngineAPI)
