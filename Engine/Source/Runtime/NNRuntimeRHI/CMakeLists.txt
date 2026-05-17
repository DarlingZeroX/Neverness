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

#开启MSVC多线程编译
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Gm->)
cmake_host_system_information(RESULT CPU_NUMBER_OF_LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_NUMBER_OF_LOGICAL_CORES}>)

add_library(NevernessRuntime-RHI SHARED)

target_sources(NevernessRuntime-RHI
  PRIVATE
    Include/OpenGL/Buffer.h
    Include/OpenGL/Core.h
    Include/OpenGL/Effect.h
    Include/OpenGL/Exception.h
    Include/OpenGL/FrameBuffer.h
    Include/OpenGL/IncludeGladGL3.h
    Include/OpenGL/InputLayout.h
    Include/OpenGL/Interface.h
    Include/OpenGL/Mesh.h
    Include/OpenGL/OpenGL.h
    Include/OpenGL/RenderTarget.h
    Include/OpenGL/Shader.h
    Include/OpenGL/ShaderProgram.h
    Include/OpenGL/Texture2D.h
    Include/OpenGL/TextureBase.h
    Include/OpenGL/ThrowMarco.h
    Include/OpenGL/VertexArray.h
    Include/VGRHIConfig.h
    Include/VGRHICore.h
    Interface/Device.h
    Interface/Mesh.h
    Interface/Shader.h
    Interface/Texture.h
    Interface/VertexElement.h
    Interface/VGFX.h
    Source/Interface/Device.cpp
    Source/Interface/VGFX.cpp
    Source/OpenGL/Buffer.cpp
    Source/OpenGL/Effect.cpp
    Source/OpenGL/Exception.cpp
    Source/OpenGL/FrameBuffer.cpp
    Source/OpenGL/IncludeGladGL3.cpp
    Source/OpenGL/Mesh.cpp
    Source/OpenGL/RenderTarget.cpp
    Source/OpenGL/Shader.cpp
    Source/OpenGL/ShaderProgram.cpp
    Source/OpenGL/Texture2D.cpp
    Source/OpenGL/TextureBase.cpp
    Source/OpenGL/VertexArray.cpp
)

target_compile_definitions(NevernessRuntime-RHI PRIVATE VG_RHI_API_EXPORT)

target_include_directories(NevernessRuntime-RHI PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)
target_include_directories(NevernessRuntime-RHI PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Core)
target_include_directories(NevernessRuntime-RHI PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)

target_link_libraries(NevernessRuntime-RHI PRIVATE SDL3::SDL3)
target_link_libraries(NevernessRuntime-RHI PRIVATE NevernessCore-Core)

if (APPLE)
    target_link_libraries(NevernessRuntime-RHI PRIVATE "-framework OpenGL")
elseif (UNIX)
    target_link_libraries(NevernessRuntime-RHI PRIVATE GL)
elseif (WIN32)
    target_link_libraries(NevernessRuntime-RHI PRIVATE opengl32)
endif ()
