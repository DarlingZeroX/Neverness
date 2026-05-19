# NNRuntimeKernelApplication — 可选 Kernel 演示（默认不加入主产品目标）。

option(NEVERNESS_BUILD_RUNTIME_KERNEL_APP "Build NNRuntimeKernelApplication demo" OFF)

if (NOT NEVERNESS_BUILD_RUNTIME_KERNEL_APP)
	return()
endif()

add_executable(NNRuntimeKernelApplication Main.cpp)

target_link_libraries(NNRuntimeKernelApplication PRIVATE
	NevernessRuntime-EngineServices
)

target_include_directories(NNRuntimeKernelApplication PRIVATE
	${PROJECT_SOURCE_DIR}/Engine/Source/Runtime/NNRuntimeEngineServices/Include
	${PROJECT_SOURCE_DIR}/Engine/Source/Runtime/NNRuntimeManagedBridge/Include
)
