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
add_definitions(-DVGCORE_DYNAMIC) 
add_definitions(-DVG_CORE_API_EXPORT) 

add_library(NevernessRuntime-Core SHARED)

target_sources(NevernessRuntime-Core
  PRIVATE
    Include/Core/Application.h
    Include/Core/CoreTypes.h
    Include/Core/EventBus.h
    Include/Core/Events.h
    Include/Core/Input.h
    Include/Core/RuntimeCore.h
    Include/Core/Viewport.h
    Include/Core/Window.h
    Include/Data/DataContainer.h
    Include/Data/DataNamespace.h
    Include/Data/DataVariant.h
    
    Include/Utils/LuaHelper.h
    Include/Utils/TimeHelper.h
    Include/Utils/TransitionHelper.h
    RuntimeCoreExport.h
    Source/Core/Application.cpp
    Source/Core/EventBus.cpp
    Source/Core/Input.cpp
    Source/Core/RuntimeCore.cpp
    Source/Core/Viewport.cpp
    Source/Core/Window.cpp
    Source/Data/DataContainer.cpp
    Source/Data/DataNamespace.cpp
    Source/Data/DataVariant.cpp
    Source/Interface/AssetInterface.cpp
    Source/Interface/EngineState.cpp
    Source/Interface/FileInterface.cpp
    Source/Interface/GameInterface.cpp
    Source/Interface/ISceneFactory.cpp
    Source/Interface/Loader.cpp
    Source/Interface/SceneInterface.cpp
    
    Source/Utils/LuaHelper.cpp
    Source/Utils/TimeHelper.cpp
    Source/Utils/TransitionHelper.cpp
)

# 添加头文件搜索路径
target_include_directories(NevernessRuntime-Core PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-Core PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-Core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-Core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
target_include_directories(NevernessRuntime-Core PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime/NNRuntimeLua/Include)
 
find_package(SDL3 CONFIG REQUIRED)
find_package(RmlUi REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

target_link_libraries(NevernessRuntime-Core PRIVATE NevernessRuntime-Lua)
target_link_libraries(NevernessRuntime-Core PUBLIC SDL3::SDL3)
target_link_libraries(NevernessRuntime-Core PRIVATE $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessCore-Core)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessCore-PlatformCore)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessCore-FileSystem)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessRuntime-VFS)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessRuntime-ImGui)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessRuntime-RHI)
target_link_libraries(NevernessRuntime-Core PRIVATE NevernessRuntime-Pak)
