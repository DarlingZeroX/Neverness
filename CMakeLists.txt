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
project ("Neverness" LANGUAGES C CXX)

############ 全局设置多线程编译（MSVC）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

############ MSVC 编译选项
if(MSVC AND TRUE)
    message(STATUS "Using MSVC")

    # 关闭警告 C4819: 无效的字符。避免中文注释被警告
    add_compile_options(
        "/wd4819"
    )

    # /showIncludes 保留：CMake 依赖扫描需要它来追踪头文件变更（增量编译）
endif()

############ 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

############ 设置输出目录
## 设置静态库文件目录
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
## 动态库文件目录
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
## 可执行文件目录
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ============================================
# 全局统一输出目录（C++ / C# / CoreCLR）
# ============================================

# 最终输出根目录
set(NN_OUTPUT_ROOT ${CMAKE_SOURCE_DIR}/Build)

# --------------------------------------------
# 默认输出目录
# --------------------------------------------

# exe / dll
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${NN_OUTPUT_ROOT}/bin)

# shared library (.dll/.so)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${NN_OUTPUT_ROOT}/bin)

# static library (.lib/.a)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    ${NN_OUTPUT_ROOT}/lib)

# pdb
if(MSVC)
    set(CMAKE_PDB_OUTPUT_DIRECTORY
        ${NN_OUTPUT_ROOT}/bin)
endif()

# --------------------------------------------
# 多配置输出目录（Debug / Release）
# --------------------------------------------

foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)

    string(TOUPPER ${CONFIG} CONFIG_UPPER)

    # EXE / DLL
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER}
        ${NN_OUTPUT_ROOT}/bin/${CONFIG})

    # Shared Library
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER}
        ${NN_OUTPUT_ROOT}/bin/${CONFIG})

    # Static Library
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER}
        ${NN_OUTPUT_ROOT}/lib/${CONFIG})

    # PDB
    if(MSVC)
        set(CMAKE_PDB_OUTPUT_DIRECTORY_${CONFIG_UPPER}
            ${NN_OUTPUT_ROOT}/bin/${CONFIG})
    endif()

endforeach()

############ 源码
add_subdirectory("Engine/Source")

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
