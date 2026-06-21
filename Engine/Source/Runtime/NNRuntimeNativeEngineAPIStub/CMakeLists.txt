# NNRuntimeNativeEngineAPIStub — Editor / Tests / ManagedHost 虛擬 Runtime（Stub + Mock）。

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	add_compile_options(/utf-8)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

add_library(NevernessRuntime-NativeEngineAPIStub STATIC)

target_sources(NevernessRuntime-NativeEngineAPIStub
	PRIVATE
		Public/NNRuntimeNativeEngineApiStub.h
		Private/Common/StubInvokeCounter.cpp
		Private/Registry/NNNativeEngineApiTable.cpp
		Private/Render/RenderApiStubs.cpp
		Private/Audio/AudioApiStubs.cpp
		Private/Input/InputApiStubs.cpp
		Private/Application/ApplicationApiStubs.cpp
		Private/Window/WindowApiStubs.cpp
		Private/Vfs/VfsApiStubs.cpp
		Private/Async/AsyncWaitApiStubs.cpp
		Private/Event/EventApiStubs.cpp
		Private/RenderAsset/RenderAssetApiStubs.cpp
		Private/ViewportRender/ViewportRenderApiStubs.cpp
)

target_include_directories(NevernessRuntime-NativeEngineAPIStub
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/Public"
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/Private"
		"${PROJECT_SOURCE_DIR}/Engine/Source/Runtime"
)

target_link_libraries(NevernessRuntime-NativeEngineAPIStub PUBLIC NevernessRuntime-NativeEngineAPI)

add_library(NNNativeEngineAPIStub ALIAS NevernessRuntime-NativeEngineAPIStub)
