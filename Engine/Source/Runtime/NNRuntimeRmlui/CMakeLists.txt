# CMakeList.txt: GalEngine 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
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

include(${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeRmluiSources.cmake)

add_library(NevernessRuntime-RmlUI SHARED)
target_sources(NevernessRuntime-RmlUI PRIVATE ${NNRuntimeRmlui_SOURCES})

target_compile_definitions(NevernessRuntime-RmlUI PRIVATE VG_UI_EXPORT)

target_include_directories(NevernessRuntime-RmlUI PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-RmlUI PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-RmlUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessRuntime-RmlUI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
target_include_directories(NevernessRuntime-RmlUI PUBLIC ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime/NNRuntimeLua/Include)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE SDL3::SDL3)

find_package(SDL3_image CONFIG REQUIRED)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE $<IF:$<TARGET_EXISTS:SDL3_image::SDL3_image-shared>,SDL3_image::SDL3_image-shared,SDL3_image::SDL3_image-static>)

find_package(RmlUi CONFIG REQUIRED)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE RmlUi::RmlUi)
 
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessCore-Core)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessCore-PlatformCore)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessCore-FileSystem)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessCore-MediaCore)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessRuntime-Lua)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessRuntime-Core)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessRuntime-RHI)
target_link_libraries(NevernessRuntime-RmlUI PRIVATE NevernessRuntime-VFS)
