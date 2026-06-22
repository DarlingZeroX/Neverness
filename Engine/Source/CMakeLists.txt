########## 测试
if (ENABLE_TESTS)
    enable_testing()
    # Legacy Galgame 測試見 Engine/Source/Legacy/CMakeLists.txt（需 NEVERNESS_BUILD_LEGACY_GALGAME=ON）
    # VGLauncherTest: add_subdirectory in NNLauncher.
endif()

############ 模块
add_subdirectory ("Core")
add_subdirectory ("Rendering")   # 必须先于 Runtime（提供 NNRuntimeRender/NNRuntimeDiligent 静态库）
add_subdirectory ("Runtime")
add_subdirectory ("Bridge")

############ 模块
add_subdirectory ("Editor")

########## 自动加载 Plugins 目录下所有子项目
file(GLOB PLUGINS_DIRS RELATIVE ${CMAKE_SOURCE_DIR}/Plugins ${CMAKE_SOURCE_DIR}/Plugins/*)
foreach(child ${PLUGINS_DIRS})
    if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/Plugins/${child}")
        add_subdirectory(${CMAKE_SOURCE_DIR}/Plugins/${child})
    endif()
endforeach()

############ Rendering（渲染模块，在 Runtime 之前编译）
# Rendering 提供 NNRuntimeRender/NNRuntimeDiligent/RmlDiligent 等静态库
# Runtime 模块需要链接这些库，因此必须先于 Runtime 构建
