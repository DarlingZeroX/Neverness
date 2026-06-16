cmake_minimum_required(VERSION 3.20)

set(MODULE_NAME "NevernessRuntime-ImageCodec")

file(GLOB_RECURSE MODULE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
)

file(GLOB_RECURSE MODULE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Private/*.cpp"
)

add_library(${MODULE_NAME} STATIC
    ${MODULE_HEADERS}
    ${MODULE_SOURCES}
)

target_include_directories(${MODULE_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Private"
)

target_compile_features(${MODULE_NAME} PUBLIC cxx_std_17)

# 开启utf-8编码支持（中文注释）
if (MSVC)
    target_compile_options(${MODULE_NAME} PRIVATE /utf-8)
endif()

# 依赖 NNCore（stb_image 头文件所在位置）
target_link_libraries(${MODULE_NAME}
    PUBLIC
        NevernessCore-Core
)

set_target_properties(${MODULE_NAME} PROPERTIES
    FOLDER "Engine/Runtime"
)
