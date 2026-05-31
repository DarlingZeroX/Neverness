# RmlUI 视口尺寸不跟随 Viewport 变化修复

> 日期：2026-05-31
> 模块：NNRuntimeRmlui / NNRuntimeEngineServices
> 状态：已修复

---

## 症状

RmlUI 内容在 Scene View 中显示，但尺寸固定为初始值（1280x720），不随 EditorViewport 实际大小变化。

## 根因

1. `RmlUIRenderer::Initialize(1280, 720)` 使用固定尺寸初始化 RmlUI Context
2. 虽然每帧调用 `SetViewport(width, height)` 更新了 Context 尺寸，但 RmlUI 文档的 body 元素不会自动填充 Context 尺寸
3. CSS `width: 100%; height: 100%` 需要父元素有明确尺寸才能生效

## 修复

### 1. 文档加载时设置尺寸

`RmlUIRenderer::Sync()` 中，文档加载后立即设置文档尺寸为当前 viewport 大小：

```cpp
auto* doc = LoadDocument(item.assetGuid);
if (doc)
{
    // 设置文档尺寸为 context 大小，使 CSS 100% 生效
    doc->SetProperty(Rml::PropertyId::Width,
        Rml::Property((float)m_ViewportWidth, Rml::Unit::PX));
    doc->SetProperty(Rml::PropertyId::Height,
        Rml::Property((float)m_ViewportHeight, Rml::Unit::PX));
    doc->Show();
    ...
}
```

### 2. 每帧更新文档尺寸

`RmlUIRenderer::RenderToTexture()` 中，每帧更新所有文档尺寸，确保 viewport 大小变化时 RmlUI 跟着缩放：

```cpp
// 每帧更新所有文档尺寸（匹配当前 viewport 大小）
for (auto& [entity, runtime] : m_Documents)
{
    if (runtime.doc && runtime.state == RmlDocState::Ready)
    {
        runtime.doc->SetProperty(Rml::PropertyId::Width,
            Rml::Property((float)m_ViewportWidth, Rml::Unit::PX));
        runtime.doc->SetProperty(Rml::PropertyId::Height,
            Rml::Property((float)m_ViewportHeight, Rml::Unit::PX));
    }
}

m_Context->Update();
m_RenderInterface->BeginFrame();
m_Context->Render();
m_RenderInterface->EndFrameNoBlit();
```

### 3. 每帧同步 Viewport 尺寸

`ViewportRenderRuntimeApi.cpp` 中，每帧调用 `SetViewport` 更新 RmlUI 渲染器尺寸：

```cpp
if (g_RmlUIRenderer && g_RmlUISystem)
{
    // 每帧更新 RmlUI 视口尺寸（匹配 EditorViewport 实际大小）
    g_RmlUIRenderer->SetViewport(width, height);
    ...
}
```

## 涉及文件

| 文件 | 改动 |
|------|------|
| `NNRuntimeRmlui/Source/Renderer/RmlUIRenderer.cpp` | `Sync()` 加载文档后设置尺寸；`RenderToTexture()` 每帧更新文档尺寸 |
| `NNRuntimeEngineServices/Private/ViewportRender/ViewportRenderRuntimeApi.cpp` | 每帧调用 `SetViewport(width, height)` |

## 教训

RmlUI 的 `Context::SetDimensions()` 只更新 context 的逻辑尺寸，不会自动改变已加载文档的元素尺寸。文档的 body 元素需要通过 `SetProperty(Width/Height)` 显式设置，CSS 百分比才能基于该尺寸计算。
