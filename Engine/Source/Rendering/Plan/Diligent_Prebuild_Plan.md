# Diligent 预编译方案（v2）

> **状态**: 待审批  
> **创建日期**: 2026-06-09  
> **目标**: 第三方库独立编译，主项目 Delete Cache 秒完成

---

## 问题

当前 `add_subdirectory(DiligentEngine)` 把 Diligent 纳入主构建树。每次 Delete Cache and Reconfigure，Diligent 全部重编（5+ 分钟）。

但更根本的问题是：**正常开发不需要 Delete Cache**。改 `.cpp` 直接 Build 即可。只有改 `CMakeLists.txt` 才需要。

---

## 方案：IMPORTED 预编译

### 原则

- Diligent 不支持 `find_package`，不写 `DiligentEngineConfig.cmake`
- 不用 `cmake --install`，Diligent 没有统一安装产物
- 直接编译一次，用 `IMPORTED` target 链接 `.lib` 文件
- 主项目的 `add_subdirectory(DiligentEngine)` 改为 `IMPORTED` 导入

### 目录结构

```
E:\Neverness\
├── Engine/
│   ├── Source/
│   │   ├── ThirdParty/
│   │   │   ├── DiligentEngine/        # 源码（只读）
│   │   │   └── nvapi_stub/            # NVAPI stub
│   │   ├── Experiments/               # Experiments 模块
│   │   └── Runtime/                   # Runtime 模块
│   └── ...
├── ExternalBuild/                     # 预编译产物（.gitignore）
│   └── Diligent/
│       └── build/                     # Diligent 编译产物
│           ├── DiligentCore/
│           │   └── Graphics/
│           │       └── GraphicsEngine/
│           │           └── Diligent-GraphicsEngine.lib
│           ├── DiligentCore/Graphics/GraphicsEngineD3D12/
│           │   └── Diligent-GraphicsEngineD3D12-static.lib
│           ├── DiligentCore/Graphics/GraphicsEngineD3D11/
│           │   └── Diligent-GraphicsEngineD3D11-static.lib
│           ├── DiligentCore/Graphics/GraphicsEngineOpenGL/
│           │   └── Diligent-GraphicsEngineOpenGL-static.lib
│           ├── DiligentCore/Graphics/GraphicsEngineVk/
│           │   └── Diligent-GraphicsEngineVk-static.lib
│           ├── DiligentCore/Graphics/GraphicsTools/
│           │   └── Diligent-GraphicsTools.lib
│           ├── DiligentCore/ShaderTools/
│           │   └── Diligent-ShaderTools.lib
│           ├── DiligentTools/Imgui/
│           │   └── Diligent-Imgui.lib
│           ├── DiligentTools/TextureLoader/
│           │   └── Diligent-TextureLoader.lib
│           └── ... (其他 .lib)
└── Build/                             # 主项目构建目录
```

### Step 1: 编译 Diligent（一次性）

创建脚本 `Engine/Source/ThirdParty/BuildDiligent.bat`：

```bat
@echo off
REM 编译 Diligent Engine（只需运行一次）

set DILIGENT_SRC=E:\Neverness\Engine\Source\ThirdParty\DiligentEngine
set DILIGENT_BUILD=E:\Neverness\ExternalBuild\Diligent\build

REM 先编译 NVAPI stub
cl /nologo /c /Fo"E:\Neverness\Engine\Source\ThirdParty\nvapi_stub\amd64\nvapi64.obj" ^
   "E:\Neverness\Engine\Source\ThirdParty\nvapi_stub\nvapi_stub.c"
lib /nologo /OUT:"E:\Neverness\Engine\Source\ThirdParty\nvapi_stub\amd64\nvapi64.lib" ^
    "E:\Neverness\Engine\Source\ThirdParty\nvapi_stub\amd64\nvapi64.obj"

REM 配置 Diligent
cmake -S "%DILIGENT_SRC%" -B "%DILIGENT_BUILD%" -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DDILIGENT_BUILD_TOOLS=OFF ^
    -DDILIGENT_BUILD_FX=OFF ^
    -DDILIGENT_BUILD_SAMPLES=OFF ^
    -DDILIGENT_NO_DLSS=ON ^
    -DDILIGENT_NO_HQ_DSR=ON ^
    -DDILIGENT_NVAPI_PATH=E:\Neverness\Engine\Source\ThirdParty\nvapi_stub

REM 编译
cmake --build "%DILIGENT_BUILD%" --config Release --parallel

echo.
echo Diligent 编译完成！产物在: %DILIGENT_BUILD%
pause
```

运行一次，产出所有 `.lib` 文件。

### Step 2: 创建 IMPORTED 模块

创建 `Engine/Source/ThirdParty/DiligentTargets.cmake`：

```cmake
# DiligentTargets.cmake — 导入预编译的 Diligent 静态库
# 不使用 add_subdirectory，不触发 Diligent 重新编译

if(TARGET Diligent-GraphicsEngine)
    return() # 已导入，跳过
endif()

set(_DILIGENT_BUILD "${CMAKE_CURRENT_SOURCE_DIR}/../../ExternalBuild/Diligent/build")
set(_DILIGENT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/DiligentEngine")

# 头文件目录（指向源码的 interface 目录）
set(DILIGENT_INCLUDE_DIRS
    "${_DILIGENT_SRC}/DiligentCore"
    "${_DILIGENT_SRC}/DiligentTools"
    "${_DILIGENT_SRC}/DiligentTools/ThirdParty/imgui"
    "${_DILIGENT_SRC}/DiligentTools/ThirdParty/imgui/backends"
    "${_DILIGENT_SRC}/DiligentTools/Imgui/interface"
)

# 辅助函数：导入一个 Diligent 静态库
function(_diligent_import_static TARGET_NAME LIB_REL_PATH)
    set(_lib "${_DILIGENT_BUILD}/${LIB_REL_PATH}")
    if(EXISTS "${_lib}")
        add_library(${TARGET_NAME} STATIC IMPORTED)
        set_target_properties(${TARGET_NAME} PROPERTIES
            IMPORTED_LOCATION "${_lib}"
            INTERFACE_INCLUDE_DIRECTORIES "${DILIGENT_INCLUDE_DIRS}"
        )
    else()
        message(WARNING "Diligent lib not found: ${_lib}")
    endif()
endfunction()

# 核心库
_diligent_import_static(Diligent-GraphicsEngine
    "DiligentCore/Graphics/GraphicsEngine/Diligent-GraphicsEngine.lib")
_diligent_import_static(Diligent-GraphicsAccessories
    "DiligentCore/Graphics/GraphicsAccessories/Diligent-GraphicsAccessories.lib")
_diligent_import_static(Diligent-GraphicsEngineD3DBase
    "DiligentCore/Graphics/GraphicsEngineD3DBase/Diligent-GraphicsEngineD3DBase.lib")
_diligent_import_static(Diligent-Common
    "DiligentCore/Common/Diligent-Common.lib")
_diligent_import_static(Diligent-Primitives
    "DiligentCore/Primitives/Diligent-Primitives.lib")
_diligent_import_static(Diligent-PlatformInterface
    "DiligentCore/Platforms/Basic/Diligent-PlatformInterface.lib")

# 平台库
if(WIN32)
    _diligent_import_static(Diligent-Win32Platform
        "DiligentCore/Platforms/Win32/Diligent-Win32Platform.lib")
endif()

# 图形后端
_diligent_import_static(Diligent-GraphicsEngineD3D12-static
    "DiligentCore/Graphics/GraphicsEngineD3D12/Diligent-GraphicsEngineD3D12-static.lib")
_diligent_import_static(Diligent-GraphicsEngineD3D11-static
    "DiligentCore/Graphics/GraphicsEngineD3D11/Diligent-GraphicsEngineD3D11-static.lib")
_diligent_import_static(Diligent-GraphicsEngineOpenGL-static
    "DiligentCore/Graphics/GraphicsEngineOpenGL/Diligent-GraphicsEngineOpenGL-static.lib")
_diligent_import_static(Diligent-GraphicsEngineVk-static
    "DiligentCore/Graphics/GraphicsEngineVk/Diligent-GraphicsEngineVk-static.lib")

# 工具库
_diligent_import_static(Diligent-GraphicsTools
    "DiligentCore/Graphics/GraphicsTools/Diligent-GraphicsTools.lib")
_diligent_import_static(Diligent-ShaderTools
    "DiligentCore/ShaderTools/Diligent-ShaderTools.lib")
_diligent_import_static(Diligent-TextureLoader
    "DiligentTools/TextureLoader/Diligent-TextureLoader.lib")
_diligent_import_static(Diligent-AssetLoader
    "DiligentTools/AssetLoader/Diligent-AssetLoader.lib")
_diligent_import_static(Diligent-Imgui
    "DiligentTools/Imgui/Diligent-Imgui.lib")

# 第三方库
_diligent_import_static(glslang
    "DiligentCore/ThirdParty/glslang/glslang/glslang.lib")
_diligent_import_static(SPIRV-Tools-opt
    "DiligentCore/ThirdParty/SPIRV-Tools/source/opt/SPIRV-Tools-opt.lib")
_diligent_import_static(SPIRV-Tools
    "DiligentCore/ThirdParty/SPIRV-Tools/source/SPIRV-Tools.lib")
_diligent_import_static(spirv-cross-core
    "DiligentCore/ThirdParty/SPIRV-Cross/spirv-cross-core/spirv-cross-core.lib")

# 平台定义
if(WIN32)
    add_compile_definitions(PLATFORM_WIN32=1)
endif()

message(STATUS "Diligent targets imported from: ${_DILIGENT_BUILD}")
```

### Step 3: 修改 Experiments CMakeLists.txt

```cmake
# 之前：
# add_subdirectory(${DILIGENT_ENGINE_DIR} ${DILIGENT_BIN_DIR})

# 之后：
include("${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/DiligentTargets.cmake")
```

一行替换。不再 `add_subdirectory`，不再触发 Diligent 重编。

### Step 4: .gitignore

```
ExternalBuild/
```

预编译产物不进 git。

---

## 效果

| 操作 | 之前 | 之后 |
|------|------|------|
| Delete Cache + Reconfigure | 5+ 分钟 | <10 秒 |
| 改 Renderer2D.cpp | 直接 Build | 直接 Build |
| 改 CMakeLists.txt | Delete Cache + 5 分钟 | Delete Cache + 10 秒 |
| 改 Diligent 源码 | 自动重编 | 重跑 BuildDiligent.bat |
| 首次克隆 | cmake configure | 先跑 BuildDiligent.bat |

---

## 正常开发流程

```
改 .cpp → Ctrl+Shift+B（或 cmake --build）→ 完成
```

不需要 Delete Cache。只有改 `CMakeLists.txt` 才需要。

---

## 迁移步骤

1. 创建 `ExternalBuild/` 目录
2. 创建 `BuildDiligent.bat` 脚本
3. 运行脚本，产出 `.lib` 文件
4. 创建 `ThirdParty/DiligentTargets.cmake`
5. 修改 Experiments CMakeLists.txt：`add_subdirectory` → `include(DiligentTargets.cmake)`
6. 测试：Delete Cache + Reconfigure 应该秒完成
7. 更新 .gitignore

---

## 扩展方向

未来可以把 SDL3、RmlUI、Assimp 等也按同样方式预编译：

```
ExternalBuild/
├── Diligent/
├── SDL3/
├── RmlUI/
└── Assimp/
```

主项目全部用 `IMPORTED` target 链接。Delete Cache 只重配 Neverness 自己的代码。
