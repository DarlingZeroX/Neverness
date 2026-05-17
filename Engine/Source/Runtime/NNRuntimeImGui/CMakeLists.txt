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

include(${CMAKE_CURRENT_SOURCE_DIR}/NNRuntimeImGuiSources.cmake)

add_library(NevernessRuntime-ImGui SHARED)
target_sources(NevernessRuntime-ImGui PRIVATE ${NNRuntimeImGui_SOURCES})

target_compile_definitions(NevernessRuntime-ImGui PRIVATE IMGUI_EXPORT)

target_include_directories(NevernessRuntime-ImGui PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-ImGui PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-ImGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

find_package(SDL3 CONFIG REQUIRED)
find_package(Freetype REQUIRED)

target_link_libraries(NevernessRuntime-ImGui PRIVATE Freetype::Freetype)
target_link_libraries(NevernessRuntime-ImGui PRIVATE SDL3::SDL3)
target_link_libraries(NevernessRuntime-ImGui PRIVATE NevernessCore-Core)
target_link_libraries(NevernessRuntime-ImGui PRIVATE NevernessCore-PlatformCore)

if (APPLE)
    target_link_libraries(NevernessRuntime-ImGui PRIVATE "-framework OpenGL")
elseif (UNIX)
    target_link_libraries(NevernessRuntime-ImGui PRIVATE GL)
elseif (WIN32)
    target_link_libraries(NevernessRuntime-ImGui PRIVATE opengl32)
endif ()
