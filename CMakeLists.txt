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
option(ENABLE_TESTS "Enable building tests" ON)

############ 全局设置多线程编译（MSVC）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

############ 项目名称
project ("Neverness" LANGUAGES C CXX)

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

# ==============================
# 全局编译优化（Debug 提速专用）
# ==============================
#if(MSVC)
#    # 全局移除 /RTC1（运行时检查，巨慢）
#    string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
#    string(REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
#
#    # 全局给所有项目追加优化参数（不影响调试 + 编译更快）
#    add_compile_options(
#        $<$<CONFIG:Debug>:/Od>       # Debug 保持不优化
#        $<$<CONFIG:Debug>:/Zi>       # 完整调试信息
#        #$<$<CONFIG:Debug>:/GR->      # 关闭 RTTI（大幅提速）
#        $<$<CONFIG:Debug>:/Gy>       # 函数级链接（更快链接）
#        $<$<CONFIG:Debug>:/GF>       # 字符串只读优化
#        $<$<CONFIG:Debug>:/Oi>       # 启用 intrinsic（更快）
#    )
#endif()

