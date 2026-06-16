# NNRuntimeManagedBridge — Native ↔ Managed RuntimeTick 桥接（无 CoreCLR）。

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(NevernessRuntime-ManagedBridge STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/Private/ManagedRuntimeBridge.cpp"
)

target_include_directories(NevernessRuntime-ManagedBridge
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/Include"
)

add_library(NNRuntimeManagedBridge ALIAS NevernessRuntime-ManagedBridge)
