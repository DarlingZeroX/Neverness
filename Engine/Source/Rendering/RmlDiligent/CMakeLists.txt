# RmlDiligent — RmlUi + Diligent Engine 集成实验模块

cmake_minimum_required(VERSION 3.21)
project(RmlDiligent LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# UTF-8 编码支持
if(MSVC)
    add_compile_options(/utf-8)
endif()

# Diligent Engine 路径
set(DILIGENT_ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/DiligentEngine")

# 保存原始 flags
set(CMAKE_CXX_FLAGS_SAVED "${CMAKE_CXX_FLAGS}")

# DiligentCore 配置
set(DILIGENT_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_FX OFF CACHE BOOL "" FORCE)
set(DILIGENT_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(DILIGENT_NO_DLSS ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_HQ_DSR ON CACHE BOOL "" FORCE)

# 禁用警告转错误
if(MSVC)
    string(REPLACE "/WX" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /w")
endif()

# 跳过 ATL 检查（系统已有 ATL）
set(SKIP_ATL_CHECK TRUE CACHE BOOL "" FORCE)

# NVAPI stub — 已移至 Rendering 主 CMakeLists.txt（必须在 Diligent 子目录之前设置）
# 禁用 SuperResolution（避免下载 GitHub 依赖）
set(DILIGENT_NO_DSR ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_DLSS ON CACHE BOOL "" FORCE)

# 先添加 Diligent Engine 子目录（定义 Diligent-* targets）
# DiligentCore — only add if not already added by parent
if(NOT TARGET Diligent-BasicPlatform)
    add_subdirectory(${DILIGENT_ENGINE_DIR} DiligentEngine)
endif()

# 查找 SDL3
find_package(SDL3 CONFIG REQUIRED)

# Phase 0 测试（无窗口，只验证 Shader + PSO）
add_executable(RmlDiligentPhase0Test Tests/TestPhase0.cpp)
target_include_directories(RmlDiligentPhase0Test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
    ${DILIGENT_ENGINE_DIR}/DiligentCore
)
target_link_libraries(RmlDiligentPhase0Test PRIVATE Diligent-GraphicsEngine)
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(RmlDiligentPhase0Test PRIVATE Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(RmlDiligentPhase0Test PRIVATE Diligent-GraphicsEngineD3D11-static)
endif()
target_compile_definitions(RmlDiligentPhase0Test PRIVATE
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
)

# Phase 1 测试（SDL3 窗口 + Smoke Test）
add_executable(RmlDiligentPhase1Test Tests/TestPhase1.cpp)
target_include_directories(RmlDiligentPhase1Test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
    ${DILIGENT_ENGINE_DIR}/DiligentCore
)
target_link_libraries(RmlDiligentPhase1Test PRIVATE
    Diligent-GraphicsEngine
    SDL3::SDL3
)
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(RmlDiligentPhase1Test PRIVATE Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(RmlDiligentPhase1Test PRIVATE Diligent-GraphicsEngineD3D11-static)
endif()
target_compile_definitions(RmlDiligentPhase1Test PRIVATE
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
)

# Phase 1.5 测试（RmlUi 集成）
find_package(RmlUi CONFIG REQUIRED)

set(RML_DILIGENT_TEST_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/TestAssets" CACHE PATH "Neverness phase test RML assets")
file(TO_CMAKE_PATH "${RML_DILIGENT_TEST_ASSETS_DIR}" RML_DILIGENT_TEST_ASSETS_DIR_CMAKE)
string(REPLACE "\\" "/" RML_DILIGENT_TEST_ASSETS_DIR_CMAKE "${RML_DILIGENT_TEST_ASSETS_DIR_CMAKE}")
if(NOT RML_DILIGENT_TEST_ASSETS_DIR_CMAKE MATCHES "/$")
    set(RML_DILIGENT_TEST_ASSETS_DIR_CMAKE "${RML_DILIGENT_TEST_ASSETS_DIR_CMAKE}/")
endif()

set(RMLUI_SUBMODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/RmlUi")
set(RMLUI_OFFICIAL_SAMPLES_DIR "${RMLUI_SUBMODULE_DIR}/Samples")
set(RMLUI_SAMPLES_DIR "${RML_DILIGENT_TEST_ASSETS_DIR}" CACHE PATH "Deprecated: use RML_DILIGENT_TEST_ASSETS_DIR for phase tests")
set(RMLUI_SAMPLES_DIR_CMAKE "${RML_DILIGENT_TEST_ASSETS_DIR_CMAKE}")

set(RML_DILIGENT_RENDER_SOURCES
    Private/RmlDiligentRenderInterface.cpp
    Renderer/RmlDiligentRenderTargetPool.cpp
    Renderer/RmlDiligentLayerStack.cpp
    Renderer/RmlDiligentSrbCache.cpp
)

set(RML_DILIGENT_RENDER_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
    ${CMAKE_CURRENT_SOURCE_DIR}/Private
    ${CMAKE_CURRENT_SOURCE_DIR}/Renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/Shaders
    ${DILIGENT_ENGINE_DIR}/DiligentCore
)

add_executable(RmlDiligentPhase1_5Test
    Tests/TestPhase1_5.cpp
    ${RML_DILIGENT_RENDER_SOURCES}
)
target_include_directories(RmlDiligentPhase1_5Test PRIVATE
    ${RML_DILIGENT_RENDER_INCLUDES}
)
target_link_libraries(RmlDiligentPhase1_5Test PRIVATE
    Diligent-GraphicsEngine
    SDL3::SDL3
    RmlUi::Core
)
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(RmlDiligentPhase1_5Test PRIVATE Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(RmlDiligentPhase1_5Test PRIVATE Diligent-GraphicsEngineD3D11-static)
endif()
target_compile_definitions(RmlDiligentPhase1_5Test PRIVATE
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
    "RMLUI_SAMPLES_DIR=\"${RMLUI_SAMPLES_DIR_CMAKE}\""
)

if(WIN32)
    set(_rml_phase15_runtime_dlls "")
    if(TARGET SDL3::SDL3)
        list(APPEND _rml_phase15_runtime_dlls "$<TARGET_FILE:SDL3::SDL3>")
    endif()
    if(TARGET RmlUi::Core)
        list(APPEND _rml_phase15_runtime_dlls "$<TARGET_FILE:RmlUi::Core>")
    endif()
    if(_rml_phase15_runtime_dlls)
        add_custom_command(TARGET RmlDiligentPhase1_5Test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${_rml_phase15_runtime_dlls}
                "$<TARGET_FILE_DIR:RmlDiligentPhase1_5Test>"
            COMMENT "Copy runtime DLLs for RmlDiligentPhase1_5Test"
        )
    endif()
endif()

# Phase 2 测试（CSS Transform）
add_executable(RmlDiligentPhase2Test
    Tests/TestTransforms.cpp
    ${RML_DILIGENT_RENDER_SOURCES}
)
target_include_directories(RmlDiligentPhase2Test PRIVATE
    ${RML_DILIGENT_RENDER_INCLUDES}
)
target_link_libraries(RmlDiligentPhase2Test PRIVATE
    Diligent-GraphicsEngine
    SDL3::SDL3
    RmlUi::Core
)
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(RmlDiligentPhase2Test PRIVATE Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(RmlDiligentPhase2Test PRIVATE Diligent-GraphicsEngineD3D11-static)
endif()
target_compile_definitions(RmlDiligentPhase2Test PRIVATE
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
    "RMLUI_SAMPLES_DIR=\"${RMLUI_SAMPLES_DIR_CMAKE}\""
)

if(WIN32)
    set(_rml_phase2_runtime_dlls "")
    if(TARGET SDL3::SDL3)
        list(APPEND _rml_phase2_runtime_dlls "$<TARGET_FILE:SDL3::SDL3>")
    endif()
    if(TARGET RmlUi::Core)
        list(APPEND _rml_phase2_runtime_dlls "$<TARGET_FILE:RmlUi::Core>")
    endif()
    if(_rml_phase2_runtime_dlls)
        add_custom_command(TARGET RmlDiligentPhase2Test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${_rml_phase2_runtime_dlls}
                "$<TARGET_FILE_DIR:RmlDiligentPhase2Test>"
            COMMENT "Copy runtime DLLs for RmlDiligentPhase2Test"
        )
    endif()
endif()

# Phase 3 测试（Gradient + Creation）
add_executable(RmlDiligentPhase3Test
    Tests/TestCustomShader.cpp
    ${RML_DILIGENT_RENDER_SOURCES}
)
target_include_directories(RmlDiligentPhase3Test PRIVATE
    ${RML_DILIGENT_RENDER_INCLUDES}
)
target_link_libraries(RmlDiligentPhase3Test PRIVATE
    Diligent-GraphicsEngine
    SDL3::SDL3
    RmlUi::Core
)
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(RmlDiligentPhase3Test PRIVATE Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(RmlDiligentPhase3Test PRIVATE Diligent-GraphicsEngineD3D11-static)
endif()
target_compile_definitions(RmlDiligentPhase3Test PRIVATE
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
    "RMLUI_SAMPLES_DIR=\"${RMLUI_SAMPLES_DIR_CMAKE}\""
)

if(WIN32)
    set(_rml_phase3_runtime_dlls "")
    if(TARGET SDL3::SDL3)
        list(APPEND _rml_phase3_runtime_dlls "$<TARGET_FILE:SDL3::SDL3>")
    endif()
    if(TARGET RmlUi::Core)
        list(APPEND _rml_phase3_runtime_dlls "$<TARGET_FILE:RmlUi::Core>")
    endif()
    if(_rml_phase3_runtime_dlls)
        add_custom_command(TARGET RmlDiligentPhase3Test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${_rml_phase3_runtime_dlls}
                "$<TARGET_FILE_DIR:RmlDiligentPhase3Test>"
            COMMENT "Copy runtime DLLs for RmlDiligentPhase3Test"
        )
    endif()
endif()

# Phase 4 测试（Clip Mask / Stencil）
add_executable(RmlDiligentPhase4Test
    Tests/TestClipMask.cpp
    ${RML_DILIGENT_RENDER_SOURCES}
)
target_include_directories(RmlDiligentPhase4Test PRIVATE
    ${RML_DILIGENT_RENDER_INCLUDES}
)
target_link_libraries(RmlDiligentPhase4Test PRIVATE
    Diligent-GraphicsEngine
    SDL3::SDL3
    RmlUi::Core
)
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(RmlDiligentPhase4Test PRIVATE Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(RmlDiligentPhase4Test PRIVATE Diligent-GraphicsEngineD3D11-static)
endif()
target_compile_definitions(RmlDiligentPhase4Test PRIVATE
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
    "RMLUI_SAMPLES_DIR=\"${RMLUI_SAMPLES_DIR_CMAKE}\""
)

if(WIN32)
    set(_rml_phase4_runtime_dlls "")
    if(TARGET SDL3::SDL3)
        list(APPEND _rml_phase4_runtime_dlls "$<TARGET_FILE:SDL3::SDL3>")
    endif()
    if(TARGET RmlUi::Core)
        list(APPEND _rml_phase4_runtime_dlls "$<TARGET_FILE:RmlUi::Core>")
    endif()
    if(_rml_phase4_runtime_dlls)
        add_custom_command(TARGET RmlDiligentPhase4Test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${_rml_phase4_runtime_dlls}
                "$<TARGET_FILE_DIR:RmlDiligentPhase4Test>"
            COMMENT "Copy runtime DLLs for RmlDiligentPhase4Test"
        )
    endif()
endif()

# Phase 5a 测试（LayerStack + RT Pool）
add_executable(RmlDiligentPhase5aTest
    Tests/TestLayerStack.cpp
    ${RML_DILIGENT_RENDER_SOURCES}
)
target_include_directories(RmlDiligentPhase5aTest PRIVATE
    ${RML_DILIGENT_RENDER_INCLUDES}
)
target_link_libraries(RmlDiligentPhase5aTest PRIVATE
    Diligent-GraphicsEngine
    SDL3::SDL3
    RmlUi::Core
)
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(RmlDiligentPhase5aTest PRIVATE Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(RmlDiligentPhase5aTest PRIVATE Diligent-GraphicsEngineD3D11-static)
endif()
target_compile_definitions(RmlDiligentPhase5aTest PRIVATE
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
    "RMLUI_SAMPLES_DIR=\"${RMLUI_SAMPLES_DIR_CMAKE}\""
)

if(WIN32)
    set(_rml_phase5a_runtime_dlls "")
    if(TARGET SDL3::SDL3)
        list(APPEND _rml_phase5a_runtime_dlls "$<TARGET_FILE:SDL3::SDL3>")
    endif()
    if(TARGET RmlUi::Core)
        list(APPEND _rml_phase5a_runtime_dlls "$<TARGET_FILE:RmlUi::Core>")
    endif()
    if(_rml_phase5a_runtime_dlls)
        add_custom_command(TARGET RmlDiligentPhase5aTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${_rml_phase5a_runtime_dlls}
                "$<TARGET_FILE_DIR:RmlDiligentPhase5aTest>"
            COMMENT "Copy runtime DLLs for RmlDiligentPhase5aTest"
        )
    endif()
endif()

# Phase 5b 测试（Filters）
add_executable(RmlDiligentPhase5bTest
    Tests/TestFilters.cpp
    ${RML_DILIGENT_RENDER_SOURCES}
)
target_include_directories(RmlDiligentPhase5bTest PRIVATE
    ${RML_DILIGENT_RENDER_INCLUDES}
)
target_link_libraries(RmlDiligentPhase5bTest PRIVATE
    Diligent-GraphicsEngine
    SDL3::SDL3
    RmlUi::Core
)
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(RmlDiligentPhase5bTest PRIVATE Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(RmlDiligentPhase5bTest PRIVATE Diligent-GraphicsEngineD3D11-static)
endif()
target_compile_definitions(RmlDiligentPhase5bTest PRIVATE
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
    "RMLUI_SAMPLES_DIR=\"${RMLUI_SAMPLES_DIR_CMAKE}\""
)

if(WIN32)
    set(_rml_phase5b_runtime_dlls "")
    if(TARGET SDL3::SDL3)
        list(APPEND _rml_phase5b_runtime_dlls "$<TARGET_FILE:SDL3::SDL3>")
    endif()
    if(TARGET RmlUi::Core)
        list(APPEND _rml_phase5b_runtime_dlls "$<TARGET_FILE:RmlUi::Core>")
    endif()
    if(_rml_phase5b_runtime_dlls)
        add_custom_command(TARGET RmlDiligentPhase5bTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${_rml_phase5b_runtime_dlls}
                "$<TARGET_FILE_DIR:RmlDiligentPhase5bTest>"
            COMMENT "Copy runtime DLLs for RmlDiligentPhase5bTest"
        )
    endif()
endif()

# Phase 6 测试（SRB 缓存性能基准）
add_executable(RmlDiligentPhase6Test
    Tests/TestPerformance.cpp
    ${RML_DILIGENT_RENDER_SOURCES}
)
target_include_directories(RmlDiligentPhase6Test PRIVATE
    ${RML_DILIGENT_RENDER_INCLUDES}
)
target_link_libraries(RmlDiligentPhase6Test PRIVATE
    Diligent-GraphicsEngine
    SDL3::SDL3
    RmlUi::Core
)
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(RmlDiligentPhase6Test PRIVATE Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(RmlDiligentPhase6Test PRIVATE Diligent-GraphicsEngineD3D11-static)
endif()
target_compile_definitions(RmlDiligentPhase6Test PRIVATE
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
    "RMLUI_SAMPLES_DIR=\"${RMLUI_SAMPLES_DIR_CMAKE}\""
)

if(WIN32)
    set(_rml_phase6_runtime_dlls "")
    if(TARGET SDL3::SDL3)
        list(APPEND _rml_phase6_runtime_dlls "$<TARGET_FILE:SDL3::SDL3>")
    endif()
    if(TARGET RmlUi::Core)
        list(APPEND _rml_phase6_runtime_dlls "$<TARGET_FILE:RmlUi::Core>")
    endif()
    if(_rml_phase6_runtime_dlls)
        add_custom_command(TARGET RmlDiligentPhase6Test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${_rml_phase6_runtime_dlls}
                "$<TARGET_FILE_DIR:RmlDiligentPhase6Test>"
            COMMENT "Copy runtime DLLs for RmlDiligentPhase6Test"
        )
    endif()
endif()

# =============================================================================
# Phase 7: 官方 RmlUi Samples（SDL3 + RmlDiligent Backend）
# =============================================================================

option(RML_DILIGENT_BUILD_OFFICIAL_SAMPLES "Build official RmlUi Samples/basic with RmlDiligent backend" ON)

set(RMLUI_BACKENDS_DIR "${RMLUI_SUBMODULE_DIR}/Backends")
set(RMLUI_SHELL_DIR "${RMLUI_SUBMODULE_DIR}/Samples/shell")

set(RML_DILIGENT_PLATFORM_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/Platform
    ${RMLUI_BACKENDS_DIR}
    ${RMLUI_SHELL_DIR}/include
)

set(RML_DILIGENT_RMLUI_LIBS RmlUi::Core)
if(TARGET RmlUi::Debugger)
    list(APPEND RML_DILIGENT_RMLUI_LIBS RmlUi::Debugger)
elseif(TARGET RmlUi::RmlUi)
    list(APPEND RML_DILIGENT_RMLUI_LIBS RmlUi::RmlUi)
endif()

function(rml_diligent_link_common TARGET)
    target_link_libraries(${TARGET} PRIVATE
        rmlui_backend_diligent
        rmlui_shell_diligent
        ${RML_DILIGENT_RMLUI_LIBS}
        SDL3::SDL3
        Diligent-GraphicsEngine
    )
    if(TARGET Diligent-GraphicsEngineD3D12-static)
        target_link_libraries(${TARGET} PRIVATE Diligent-GraphicsEngineD3D12-static)
    endif()
    if(TARGET Diligent-GraphicsEngineD3D11-static)
        target_link_libraries(${TARGET} PRIVATE Diligent-GraphicsEngineD3D11-static)
    endif()
    target_compile_definitions(${TARGET} PRIVATE
        PLATFORM_WIN32=1
        D3D12_SUPPORTED=1
        D3D11_SUPPORTED=1
        RMLUI_SDL_VERSION_MAJOR=3
    )
    if(MSVC)
        target_compile_options(${TARGET} PRIVATE /utf-8)
    endif()
endfunction()

function(rml_diligent_copy_samples_runtime TARGET)
    if(NOT WIN32)
        return()
    endif()
    set(_dlls "")
    if(TARGET SDL3::SDL3)
        list(APPEND _dlls "$<TARGET_FILE:SDL3::SDL3>")
    endif()
    foreach(_lib IN LISTS RML_DILIGENT_RMLUI_LIBS)
        if(TARGET ${_lib})
            list(APPEND _dlls "$<TARGET_FILE:${_lib}>")
        endif()
    endforeach()
    if(_dlls)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_dlls} "$<TARGET_FILE_DIR:${TARGET}>"
            COMMENT "Copy runtime DLLs for ${TARGET}"
        )
    endif()
    if(EXISTS "${RMLUI_OFFICIAL_SAMPLES_DIR}")
        # Shell::FindSamplesRoot() expects assets/rml.rcss next to the executable (see PlatformExtensions Win32).
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${RMLUI_OFFICIAL_SAMPLES_DIR}" "$<TARGET_FILE_DIR:${TARGET}>"
            COMMENT "Copy official RmlUi Samples tree for ${TARGET}"
        )
    endif()
endfunction()

add_library(rmlui_backend_diligent STATIC
    Platform/RmlDiligent_AppCommon.cpp
    Platform/RmlDiligent_Backend.cpp
    Platform/RmlDiligent_SystemSDLUnity.cpp
    ${RML_DILIGENT_RENDER_SOURCES}
)
target_include_directories(rmlui_backend_diligent PUBLIC
    ${RML_DILIGENT_RENDER_INCLUDES}
    ${RML_DILIGENT_PLATFORM_INCLUDES}
)
target_link_libraries(rmlui_backend_diligent PUBLIC SDL3::SDL3 Diligent-GraphicsEngine ${RML_DILIGENT_RMLUI_LIBS})
if(TARGET Diligent-GraphicsEngineD3D12-static)
    target_link_libraries(rmlui_backend_diligent PUBLIC Diligent-GraphicsEngineD3D12-static)
endif()
if(TARGET Diligent-GraphicsEngineD3D11-static)
    target_link_libraries(rmlui_backend_diligent PUBLIC Diligent-GraphicsEngineD3D11-static)
endif()
target_compile_definitions(rmlui_backend_diligent PUBLIC
    PLATFORM_WIN32=1
    D3D12_SUPPORTED=1
    D3D11_SUPPORTED=1
    RMLUI_SDL_VERSION_MAJOR=3
)

add_library(rmlui_shell_diligent STATIC
    ${RMLUI_SHELL_DIR}/src/PlatformExtensions.cpp
    ${RMLUI_SHELL_DIR}/src/RendererExtensions.cpp
    ${RMLUI_SHELL_DIR}/src/Shell.cpp
    ${RMLUI_SHELL_DIR}/src/ShellFileInterface.cpp
)
target_include_directories(rmlui_shell_diligent PUBLIC
    ${RMLUI_SHELL_DIR}/include
    ${RMLUI_BACKENDS_DIR}
)
target_link_libraries(rmlui_shell_diligent PUBLIC rmlui_backend_diligent ${RML_DILIGENT_RMLUI_LIBS})
target_compile_definitions(rmlui_shell_diligent PRIVATE RMLUI_PLATFORM_WIN32=1)
if(WIN32)
    target_link_libraries(rmlui_shell_diligent PRIVATE Shlwapi)
endif()

if(RML_DILIGENT_BUILD_OFFICIAL_SAMPLES AND EXISTS "${RMLUI_OFFICIAL_SAMPLES_DIR}/basic")
    set(_rml_basic_samples
        bitmap_font
        animation
        benchmark
        custom_log
        data_binding
        demo
        drag
        effects
        load_document
        transform
        tree_view
    )

    function(rml_diligent_add_basic_sample SAMPLE_NAME)
        set(SAMPLE_DIR "${RMLUI_OFFICIAL_SAMPLES_DIR}/basic/${SAMPLE_NAME}")
        if(NOT EXISTS "${SAMPLE_DIR}/src/main.cpp")
            message(STATUS "RmlDiligent: skip sample '${SAMPLE_NAME}' (no main.cpp)")
            return()
        endif()
        file(GLOB _sample_sources CONFIGURE_DEPENDS "${SAMPLE_DIR}/src/*.cpp" "${SAMPLE_DIR}/src/*.h")
        set(TARGET_NAME "RmlDiligent_sample_${SAMPLE_NAME}")
        add_executable(${TARGET_NAME} ${_sample_sources})
        target_include_directories(${TARGET_NAME} PRIVATE
            ${SAMPLE_DIR}/src
            ${RMLUI_SHELL_DIR}/include
            ${RMLUI_BACKENDS_DIR}
        )
        rml_diligent_link_common(${TARGET_NAME})
        rml_diligent_copy_samples_runtime(${TARGET_NAME})
        if(WIN32)
            set_target_properties(${TARGET_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
        endif()
    endfunction()

    foreach(_sample IN LISTS _rml_basic_samples)
        rml_diligent_add_basic_sample(${_sample})
    endforeach()

    # Optional plugin samples
    if(TARGET Freetype::Freetype AND TARGET harfbuzz::harfbuzz AND EXISTS "${RMLUI_OFFICIAL_SAMPLES_DIR}/basic/harfbuzz")
        file(GLOB _hb_sources "${RMLUI_OFFICIAL_SAMPLES_DIR}/basic/harfbuzz/src/*.cpp" "${RMLUI_OFFICIAL_SAMPLES_DIR}/basic/harfbuzz/src/*.h")
        add_executable(RmlDiligent_sample_harfbuzz ${_hb_sources})
        target_include_directories(RmlDiligent_sample_harfbuzz PRIVATE
            "${RMLUI_OFFICIAL_SAMPLES_DIR}/basic/harfbuzz/src"
            ${RMLUI_SHELL_DIR}/include
            ${RMLUI_BACKENDS_DIR}
        )
        rml_diligent_link_common(RmlDiligent_sample_harfbuzz)
        target_link_libraries(RmlDiligent_sample_harfbuzz PRIVATE Freetype::Freetype harfbuzz::harfbuzz)
        rml_diligent_copy_samples_runtime(RmlDiligent_sample_harfbuzz)
        if(WIN32)
            set_target_properties(RmlDiligent_sample_harfbuzz PROPERTIES WIN32_EXECUTABLE TRUE)
        endif()
    else()
        message(STATUS "RmlDiligent: harfbuzz sample skipped (Freetype/harfbuzz not found)")
    endif()

    # ime sample needs extra Win32 font sources; skipped on SDL3+Diligent until wired up.
    if(FALSE AND EXISTS "${RMLUI_OFFICIAL_SAMPLES_DIR}/basic/ime/src/main.cpp")
        message(STATUS "RmlDiligent: ime sample disabled (requires SystemFontWin32 helper sources)")
    endif()
endif()

# Phase 7 smoke test
add_executable(RmlDiligentSamplesSmokeTest Tests/TestSamplesSmoke.cpp)
target_include_directories(RmlDiligentSamplesSmokeTest PRIVATE ${RML_DILIGENT_PLATFORM_INCLUDES})
rml_diligent_link_common(RmlDiligentSamplesSmokeTest)
rml_diligent_copy_samples_runtime(RmlDiligentSamplesSmokeTest)

# 恢复原始 flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVED}")
