set(CMAKE_CXX_STANDARD 17)

add_library(NevernessRuntime-MediaAssets SHARED)

target_sources(NevernessRuntime-MediaAssets PRIVATE
    Source/NNMediaCooker.cpp
    # TODO: 添加 MediaImportAPI.cpp（C ABI 导出函数实现）
)

target_compile_definitions(NevernessRuntime-MediaAssets
    PRIVATE NN_RUNTIME_MEDIA_ASSETS_EXPORT NN_RUNTIME_MEDIA_ASSETS_DYNAMIC)

target_include_directories(NevernessRuntime-MediaAssets PUBLIC
    ${PROJECT_SOURCE_DIR}/Engine/Source/Runtime
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

target_link_libraries(NevernessRuntime-MediaAssets PUBLIC
    NevernessCore-Core
    NevernessCore-MediaCore
    NevernessRuntime-Asset
)
