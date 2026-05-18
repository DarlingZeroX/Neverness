# NNNativeEngineAPI — Native Engine Service ABI（僅頭文件契約層，無 Stub 實作）。

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(NevernessRuntime-NativeEngineAPI INTERFACE)

target_include_directories(NevernessRuntime-NativeEngineAPI
	INTERFACE
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
)

add_library(NNNativeEngineAPI ALIAS NevernessRuntime-NativeEngineAPI)
