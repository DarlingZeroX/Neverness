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

#开启MSVC多线程编译	# windows 并行编译选项
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/Gm->) #高版本已被废弃，但是低版本的Gm会影响并行
cmake_host_system_information(RESULT CPU_NUMBER_OF_LOGICAL_CORES QUERY NUMBER_OF_LOGICAL_CORES)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP${CPU_NUMBER_OF_LOGICAL_CORES}>)

# 添加宏
add_definitions(-DH_MEDIA_API_EXPORT) 

add_library(NevernessCore-MediaCore SHARED)

target_sources(NevernessCore-MediaCore
  PRIVATE
    Include/Audio.h
    Include/Audio/AudioRingBuffer.h
    Include/Audio/FAudioDecoder.h
    Include/Common/FFmpegIOContext.h
    Include/Common/InMemoryRemuxer.h
    Include/FFmpeg/FBuffer.h
    Include/FFmpeg/FChannelLayout.h
    Include/FFmpeg/FCodecContext.h
    Include/FFmpeg/FCodecParameters.h
    Include/FFmpeg/FContext.h
    Include/FFmpeg/FDictionary.h
    Include/FFmpeg/FFormatContext.h
    Include/FFmpeg/FFrame.h
    Include/FFmpeg/FIOContext.h
    Include/FFmpeg/FPacket.h
    Include/FFmpeg/FSwrContext.h
    Include/FFmpeg/FSwsContext.h
    Include/FVideo.h
    Include/HMediaConfig.h
    Include/SDL/SDLAudio.h
    Include/Video/FVideoDecoder.h
    Interface/AudioDecoderInterface.h
    Interface/AudioInterface.h
    Interface/VideoDecoderInterface.h
    Interface/VideoInterface.h
    Source/Audio.cpp
    Source/Audio/AudioRingBuffer.cpp
    Source/Audio/FAudioDecoder.cpp
    Source/Common/FFmpegIOContext.cpp
    Source/Common/InMemoryRemuxer.cpp
    Source/FFmpeg/FBuffer.cpp
    Source/FFmpeg/FChannelLayout.cpp
    Source/FFmpeg/FCodecContext.cpp
    Source/FFmpeg/FCodecParameters.cpp
    Source/FFmpeg/FContext.cpp
    Source/FFmpeg/FDictionary.cpp
    Source/FFmpeg/FFormatContext.cpp
    Source/FFmpeg/FFrame.cpp
    Source/FFmpeg/FIOContext.cpp
    Source/FFmpeg/FPacket.cpp
    Source/FFmpeg/FSwrContext.cpp
    Source/FFmpeg/FSwsContext.cpp
    Source/FVideo.cpp
    Source/SDL/SDLAudio.cpp
    Source/Video/FVideoDecoder.cpp
)

# 添加头文件搜索路径
target_include_directories(NevernessCore-MediaCore PUBLIC ${VISIONGAL_KERNEL_ROOT})
target_include_directories(NevernessCore-MediaCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(NevernessCore-MediaCore PRIVATE ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime)

# FFmpeg
find_package(FFMPEG REQUIRED)
target_include_directories(NevernessCore-MediaCore PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(NevernessCore-MediaCore PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(NevernessCore-MediaCore PRIVATE ${FFMPEG_LIBRARIES})

target_link_libraries(NevernessCore-MediaCore PRIVATE NevernessCore-Core)
target_link_libraries(NevernessCore-MediaCore PRIVATE NevernessRuntime-VFS)
