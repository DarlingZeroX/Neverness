# nvapi_stub — NVAPI 函数 stub 实现
# 供 Diligent D3D12 后端链接，阻止从 GitHub 下载真正的 NVAPI
# stub 的 NvAPI_Initialize() 返回 NVAPI_ERROR，运行时走标准 D3D 路径

cmake_minimum_required(VERSION 3.20)
project(nvapi_stub LANGUAGES C CXX)

set(_NVAPI_LIB "${CMAKE_CURRENT_SOURCE_DIR}/amd64/nvapi64.lib")
set(_NVAPI_OBJ "${CMAKE_CURRENT_SOURCE_DIR}/amd64/nvapi64.obj")
set(_NVAPI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/nvapi_stub.c")

# 配置阶段预编译：Diligent 在 IMPORTED_LIB 中引用 nvapi64.lib，必须配置时就存在
if(NOT EXISTS "${_NVAPI_LIB}")
    message(STATUS "[nvapi_stub] 预编译 nvapi64.lib ...")

    # 编译 .c → .obj
    execute_process(
        COMMAND "${CMAKE_C_COMPILER}" /nologo /c /Fo"${_NVAPI_OBJ}" "${_NVAPI_SRC}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE _rc
    )
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "[nvapi_stub] 编译失败 (rc=${_rc})")
    endif()

    # 打包 .obj → .lib
    find_program(_LIB_TOOL lib)
    if(NOT _LIB_TOOL)
        # MSVC 的 lib 在 VCINSTALLDIR/bin 里，cmake 可能找不到，用 cl 同目录
        get_filename_component(_CL_DIR "${CMAKE_C_COMPILER}" DIRECTORY)
        find_program(_LIB_TOOL lib HINTS "${_CL_DIR}")
    endif()
    execute_process(
        COMMAND "${_LIB_TOOL}" /nologo /OUT:"${_NVAPI_LIB}" "${_NVAPI_OBJ}"
        RESULT_VARIABLE _rc
    )
    if(NOT _rc EQUAL 0)
        message(FATAL_ERROR "[nvapi_stub] 打包 lib 失败 (rc=${_rc})")
    endif()

    message(STATUS "[nvapi_stub] nvapi64.lib 已生成: ${_NVAPI_LIB}")
endif()

# 同时注册为 CMake target（供增量构建和依赖追踪）
add_library(nvapi_stub_target STATIC "${_NVAPI_SRC}")
target_include_directories(nvapi_stub_target PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(nvapi_stub_target PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/amd64"
    ARCHIVE_OUTPUT_NAME "nvapi64"
)
