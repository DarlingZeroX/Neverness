# 纹理加载修复——解决显示 font atlas 问题

## 问题描述

TextureAssetOpener 打开纹理资产、ContentBrowser 拖拽纹理到 SpriteRendererInspector 后，
显示的都是 ImGui 的默认 font atlas 纹理（io.Fonts->TexRef），而非实际纹理内容。

## 根因分析

C++ `Texture2D::CreateFromMemoryImp`（`Texture2D.cpp:105`）**始终返回 `false`**，
且 `CreateFromMemory` 忽略返回值、始终返回非 null 的 `shared_ptr`，导致：

1. `UploadTextureInternal` 不会感知创建失败
2. Release 模式下 `GL_THROW_INFO(x)` 宏直接展开为 `x`，无 GL 错误检查
3. 若 `glGenTextures` 或 `glTexImage2D` 静默失败，`m_RendererID` 为 0
4. `GetShaderResourceView()` 返回 `(void*)0` → `GetImGuiTextureHandle` 返回 0
5. `ImTextureRef(null, 0)` 渲染当前绑定的 font atlas 纹理

## 修复内容

**文件**: `Runtime/NNRuntimeRHI/Source/OpenGL/Texture2D.cpp`

### 1. `CreateFromMemoryImp` 修复

- 添加参数校验：`!desc.Data || desc.Width <= 0 || desc.Height <= 0` 提前返回 false
- 添加 GL 错误检查：`glTexImage2D` 后显式调用 `glGetError()`，失败时返回 false
- 修复返回值：`return false` → `return true`

### 2. `CreateFromMemory` 修复

检查 `CreateFromMemoryImp` 返回值，失败时返回 `nullptr`：
```cpp
if (!Tex->CreateFromMemoryImp(desc))
    return nullptr;
```

`UploadTextureInternal` 中已有 `if (!glTexture)` 的检查，修复后能正确报错。

### 3. 添加 include

添加 `#include <NNCore/Interface/HLog.h>` 以支持 `H_LOG_ERROR` 宏。

## 设计决策

- **不改 `GL_THROW_INFO` 全局宏**：Release 模式下该宏被多个 GL 调用使用，波及面大。仅在 `CreateFromMemoryImp` 这个关键路径上显式调用 `glGetError()`。
- **最小改动原则**：只修复根因 bug，不引入额外抽象或重构。

## 验证方式

1. 编译 C++ 项目（`NNRuntimeRHI`、`NNRuntimeRenderAssets`）
2. 编译 C# 项目（`Neverness.Editor.Assets`、`Neverness.Editor.Scene`）
3. ContentBrowser 双击纹理资产 → TextureAssetOpener 显示实际纹理
4. ContentBrowser 拖拽纹理到 SpriteRendererInspector → 预览区显示纹理
5. 控制台日志无 `[Texture2D] glTexImage2D GL error` 输出

---

*文档版本：v1.0*
*最后更新：2026/05/27*
