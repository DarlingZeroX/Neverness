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

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(NevernessBridge-ImGui SHARED)

target_sources(NevernessBridge-ImGui
  PRIVATE
    Include/cimconfig.h
    Include/cimgui.h
    Include/cimgui_impl.h
    Source/cimgui.cpp
    Source/cimgui_impl.cpp
)

target_compile_definitions(NevernessBridge-ImGui PRIVATE NN_BRIDGE_IMGUI_EXPORT)

target_include_directories(NevernessBridge-ImGui PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessBridge-ImGui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_link_libraries(NevernessBridge-ImGui PRIVATE NevernessRuntime-ImGui)
