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

#开启MSVC多线程编译	# windows 并行编译选项
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Gm->) #高版本已被废弃，但是低版本的Gm会影响并行
cmake_host_system_information(RESULT CPU_NUMBER_OF_LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_NUMBER_OF_LOGICAL_CORES}>)

add_library(NevernessRuntime-Media SHARED)

target_sources(NevernessRuntime-Media PRIVATE
    Include/MediaRuntimeAPI.h
    Include/NNAudioClipAsset.h
    Include/NNAudioExport.h
    Include/NNAudioPlayer.h
    Include/NNVideoClipAsset.h
    Include/NNVideoPlayer.h
    Include/NNVideoTexture.h
    Source/NNVideoTexture.cpp
    Source/NNAudioExport.cpp
    # TODO: 添加以下文件的实现
    # Source/NNAudioClipAsset.cpp
    # Source/NNVideoClipAsset.cpp
    # Source/NNVideoPlayer.cpp
)

target_compile_definitions(NevernessRuntime-Media
    PRIVATE NN_RUNTIME_MEDIA_EXPORT NN_RUNTIME_MEDIA_DYNAMIC)

target_include_directories(NevernessRuntime-Media PUBLIC
    ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

target_link_libraries(NevernessRuntime-Media PUBLIC
    NevernessCore-Core
    NevernessCore-MediaCore
    NevernessRuntime-VFS
)
