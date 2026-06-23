# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

# 开启utf-8编码支持
if (MSVC)
    add_compile_options(/utf-8)
endif()
 
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif()

# 添加宏
#add_definitions(-DVGCORE_DYNAMIC) 
add_definitions(-DVG_CORE_API_EXPORT) 

add_library(NevernessRuntime-Core STATIC)

target_sources(NevernessRuntime-Core
  PRIVATE
    
    Include/Core/CoreTypes.h
    Include/Core/EventBus.h
    Include/Core/Events.h
    Include/Core/Input.h
    Include/Core/RuntimeCore.h
    Include/Core/Viewport.h

    NNObject.h
    Include/Handle/NNHandleTypes.h
    Include/Handle/NNObjectHandleRegistry.h
    
    Include/Utils/TimeHelper.h
    Include/Utils/TransitionHelper.h
    RuntimeCoreExport.h
    
    Source/Core/EventBus.cpp
    Source/Core/Input.cpp
    Source/Core/RuntimeCore.cpp
    Source/Core/Viewport.cpp
    
    Source/Utils/TimeHelper.cpp
    Source/Utils/TransitionHelper.cpp
)

# 添加头文件搜索路径
# PUBLIC Runtime/: <NNRuntimeCore/NNObject.h>
# PUBLIC Include/: <NNRuntimeCore/Handle/NNHandleTypes.h>
target_include_directories(NevernessRuntime-Core PUBLIC ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-Core PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-Core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
 
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessCore-Core)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessCore-PlatformCore)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessRuntime-VFS)

# 兼容旧的 NNRuntimeCore 目标名（Rendering 模块使用）
add_library(NNRuntimeCore INTERFACE)
target_link_libraries(NNRuntimeCore INTERFACE NevernessRuntime-Core)
