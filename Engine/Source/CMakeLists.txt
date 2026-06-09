########## 测试
if (ENABLE_TESTS)
    enable_testing()
    # Legacy Galgame 測試見 Engine/Source/Legacy/CMakeLists.txt（需 NEVERNESS_BUILD_LEGACY_GALGAME=ON）
    # VGLauncherTest: add_subdirectory in NNLauncher.
endif()

############ 模块
add_subdirectory ("Core")
add_subdirectory ("Experiments")  # 必须先于 Runtime（提供 NNRuntimeRender/NNRuntimeDiligent 静态库）
add_subdirectory ("Runtime")
add_subdirectory ("Bridge")

############ Legacy 兼容层（预设不编译）
# Phase 5：Legacy Galgame 產品線（預設不編譯）
option(NEVERNESS_BUILD_LEGACY_GALGAME "Build Legacy Galgame stack (Frozen / Compatibility Only)" OFF)
if(NEVERNESS_BUILD_LEGACY_GALGAME)
	add_subdirectory("Legacy")
endif()

############ 模块
add_subdirectory ("Editor")
#add_subdirectory ("Application")



########## 自动加载 Plugins 目录下所有子项目
file(GLOB PLUGINS_DIRS RELATIVE ${CMAKE_SOURCE_DIR}/Plugins ${CMAKE_SOURCE_DIR}/Plugins/*)
foreach(child ${PLUGINS_DIRS})
    if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/Plugins/${child}")
        add_subdirectory(${CMAKE_SOURCE_DIR}/Plugins/${child})
    endif()
endforeach()

add_subdirectory ("Host")

############ Experiments（实验模块，已移至 Runtime 之前无条件编译）
# Experiments 提供 NNRuntimeRender/NNRuntimeDiligent/RmlDiligent 等静态库
# Runtime 模块需要链接这些库，因此必须先于 Runtime 构建
# 原 option(NEVERNESS_BUILD_EXPERIMENTS) 已移除，改为无条件编译
