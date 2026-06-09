# BuildDiligent.cmake — 独立预编译 Diligent Engine
#
# 用法（在 Neverness 根目录）：
#   cmake -P Engine/Source/ThirdParty/BuildDiligent.cmake
#
# 或在 VS 中：CMake → 运行 CMake 脚本 → 选择此文件
#
# 产出：
#   Build/ThirdParty/Diligent/lib/   — 静态库
#   Build/ThirdParty/Diligent/cmake/ — CMake 配置文件（find_package 用）

cmake_minimum_required(VERSION 3.20)

set(DILIGENT_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/DiligentEngine")
set(DILIGENT_BUILD_DIR "${CMAKE_CURRENT_LIST_DIR}/../../Build/ThirdParty/Diligent/build")
set(DILIGENT_INSTALL_DIR "${CMAKE_CURRENT_LIST_DIR}/../../Build/ThirdParty/Diligent/install")

message(STATUS "=== Diligent Engine 预编译 ===")
message(STATUS "  源码: ${DILIGENT_SRC_DIR}")
message(STATUS "  构建: ${DILIGENT_BUILD_DIR}")
message(STATUS "  安装: ${DILIGENT_INSTALL_DIR}")

# 配置
execute_process(
    COMMAND ${CMAKE_COMMAND}
        -S "${DILIGENT_SRC_DIR}"
        -B "${DILIGENT_BUILD_DIR}"
        -G "Ninja"
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX=${DILIGENT_INSTALL_DIR}
        -DDILIGENT_BUILD_TOOLS=OFF
        -DDILIGENT_BUILD_FX=OFF
        -DDILIGENT_BUILD_SAMPLES=OFF
        -DDILIGENT_BUILD_DOCS=OFF
        -DDILIGENT_NO_DLSS=ON
        -DDILIGENT_NO_HQ_DSR=ON
        -DFETCHCONTENT_FULLY_DISCONNECTED=ON
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Diligent CMake 配置失败")
endif()

# 编译
execute_process(
    COMMAND ${CMAKE_COMMAND} --build "${DILIGENT_BUILD_DIR}" --config Release --parallel
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Diligent 编译失败")
endif()

# 安装
execute_process(
    COMMAND ${CMAKE_COMMAND} --install "${DILIGENT_BUILD_DIR}" --config Release
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Diligent 安装失败")
endif()

message(STATUS "=== Diligent Engine 预编译完成 ===")
message(STATUS "  安装目录: ${DILIGENT_INSTALL_DIR}")
message(STATUS "  在主项目 CMake 中使用:")
message(STATUS "    find_package(DiligentCore REQUIRED)")
message(STATUS "    target_link_libraries(... Diligent-GraphicsEngine)")
