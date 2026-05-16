# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置,并包含子项目。
cmake_minimum_required (VERSION 3.21)

############ 设置标准
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

############ 设置 Vcpkg 工具链文件，仅在用户没指定 CMAKE_TOOLCHAIN_FILE 时设置默认值
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "E:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

############ 启用测试选项
#option(ENABLE_TESTS "Enable building tests" OFF)
option(ENABLE_TESTS "Enable building tests" ON)

############ 项目名称
project ("VisionGal" LANGUAGES C CXX)

############ 全局设置多线程编译（MSVC）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

############ 强制关闭 VS 头文件包含输出（终极版）
if(MSVC AND FALSE)
    message(STATUS "Using MSVC")

    # 关闭警告 C4819: 无效的字符。避免中文注释被警告
    add_compile_options(
        "/wd4819"
    )

    # 关闭 /showIncludes
    string(REPLACE "/showIncludes" "" CMAKE_DEPFILE_FLAGS_C "${CMAKE_DEPFILE_FLAGS_C}")
    string(REPLACE "/showIncludes" "" CMAKE_DEPFILE_FLAGS_CXX "${CMAKE_DEPFILE_FLAGS_CXX}")
endif()

############ 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

############ 设置输出目录
# 设置静态库文件目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 动态库文件目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 可执行文件目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

############ Runtime模块
# 基础框架
add_subdirectory ("Engine/Source/Runtime/HCore")
add_subdirectory ("Engine/Source/Runtime/HCorePlatform")
add_subdirectory ("Engine/Source/Runtime/HFileSystem")
add_subdirectory ("Engine/Source/Runtime/HNGRuntimeCore")
add_subdirectory ("Engine/Source/Runtime/HNGRuntimeNodes")
add_subdirectory ("Engine/Source/Runtime/HMedia")

# 引擎
add_subdirectory ("Engine/Source/Runtime/VGLua")
add_subdirectory ("Engine/Source/Runtime/VGRHI")
add_subdirectory ("Engine/Source/Runtime/VGPackage")
add_subdirectory ("Engine/Source/Runtime/VGImgui")
add_subdirectory ("Engine/Source/Runtime/VGCore")
add_subdirectory ("Engine/Source/Runtime/VGNativeEngineAPI")

option(VISIONGAL_USE_ENGINE_RUNTIME_SERVICES "Use VGEngineRuntimeServices for VGNativeApi engineServices (else Stub-only table)" ON)
add_subdirectory ("Engine/Source/Runtime/VGEngineRuntime")
add_subdirectory ("Engine/Source/Runtime/VGEngineRuntimeServices")
add_subdirectory ("Engine/Source/Runtime/VGAsset")
add_subdirectory ("Engine/Source/Runtime/VGUI")
add_subdirectory ("Engine/Source/Runtime/VGEngine")

############ 托管代码（CoreCLR via nethost，预设仅 Windows/MSVC 平台启用）
if (WIN32 AND MSVC)
	option(VISIONGAL_ENABLE_MANAGED_HOST "Build VGManagedHost (CoreCLR via nethost)" ON)
else()
	option(VISIONGAL_ENABLE_MANAGED_HOST "Build VGManagedHost (CoreCLR via nethost)" OFF)
endif()

if (VISIONGAL_ENABLE_MANAGED_HOST)
	add_subdirectory ("Engine/Source/Managed/VGManagedCore")
	add_subdirectory ("Engine/Source/Managed/VGManagedHost")
endif()

############ Legacy 兼容层（预设不编译）
# Phase 5：Legacy Galgame 產品線（預設不編譯）
option(VISIONGAL_BUILD_LEGACY_GALGAME "Build Legacy Galgame stack (Frozen / Compatibility Only)" OFF)
if(VISIONGAL_BUILD_LEGACY_GALGAME)
	add_subdirectory("Engine/Source/Legacy")
endif()

############ 编辑器（主線；Galgame 編輯器模組已移至 Legacy/）
add_subdirectory ("Engine/Source/Editor/HNGEditorCore")
add_subdirectory ("Engine/Source/Editor/VGEditorFramework")
add_subdirectory ("Engine/Source/Editor/VGEditorReactive")
add_subdirectory ("Engine/Source/Editor/VGEditorComponentSchema")
add_subdirectory ("Engine/Source/Editor/VGEditorAuthoringGraph")
add_subdirectory ("Engine/Source/Editor/VGEditorExtensions")
add_subdirectory ("Engine/Source/Editor/VGEditorRuntimeBridge")
add_subdirectory ("Engine/Source/Editor/VGEditor")

############ 应用
add_subdirectory ("Engine/Source/Application/VGLauncher")         # 启动器
add_subdirectory ("Engine/Source/Application/VGDesktopApplication")   # 桌面应用程序
add_subdirectory ("Engine/Source/Application/VGPackageTool")      # 打包工具
if(VISIONGAL_BUILD_LEGACY_GALGAME)
	add_subdirectory ("Engine/Source/Application/VGNodeGraphApp")     # 节点图应用程序（Legacy Galgame）
endif()
#add_subdirectory ("Engine/Source/Application/HNGRuntimeCoreDemo")     # 节点图应用程序


########## 加载 Tests 目录下所有子测试项目
if (ENABLE_TESTS)
    enable_testing()
    # Legacy Galgame 測試見 Engine/Source/Legacy/CMakeLists.txt（需 VISIONGAL_BUILD_LEGACY_GALGAME=ON）
    # VGManagedHostTest: add_subdirectory trong VGManagedHost/CMakeLists.txt (ENABLE_TESTS + dotnet).
    add_subdirectory("Engine/Source/Managed/Tests")
      
    # add_subdirectory("Tests/VGLauncherTest")
    # add_subdirectory("Engine/Source/Tests/HNodeGraphTest")
    #file(GLOB TESTS_DIRS RELATIVE ${CMAKE_SOURCE_DIR}/Tests ${CMAKE_SOURCE_DIR}/Tests/*)
    #foreach(child ${TESTS_DIRS})
    #    if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/Tests/${child}")
    #        add_subdirectory(${CMAKE_SOURCE_DIR}/Tests/${child})
    #    endif()
    #endforeach()
endif()

########## 自动加载 Plugins 目录下所有子项目
file(GLOB PLUGINS_DIRS RELATIVE ${CMAKE_SOURCE_DIR}/Plugins ${CMAKE_SOURCE_DIR}/Plugins/*)
foreach(child ${PLUGINS_DIRS})
    if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/Plugins/${child}")
        add_subdirectory(${CMAKE_SOURCE_DIR}/Plugins/${child})
    endif()
endforeach()

############ 或者全局设置
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

############ 编译时间输出设置
if(MSVC)
    # 1. MSVC 输出每个文件：预处理/编译/优化/代码生成耗时
    add_compile_options(/time)
    # 2. 链接器输出链接总耗时
    add_link_options(/TIME)
endif()

# 3. Ninja 输出每条命令执行耗时（关键！）
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "")
# 强制 Ninja 打印每条命令的执行时间
set(NINJA_STATUS "[%f/%t] %s %es ")

# ==============================
# 全局编译优化（Debug 提速专用）
# ==============================
if(MSVC)
    # 全局移除 /RTC1（运行时检查，巨慢）
    string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

    # 全局给所有项目追加优化参数（不影响调试 + 编译更快）
    add_compile_options(
        $<$<CONFIG:Debug>:/Od>       # Debug 保持不优化
        $<$<CONFIG:Debug>:/Zi>       # 完整调试信息
        #$<$<CONFIG:Debug>:/GR->      # 关闭 RTTI（大幅提速）
        $<$<CONFIG:Debug>:/Gy>       # 函数级链接（更快链接）
        $<$<CONFIG:Debug>:/GF>       # 字符串只读优化
        $<$<CONFIG:Debug>:/Oi>       # 启用 intrinsic（更快）
    )
endif()