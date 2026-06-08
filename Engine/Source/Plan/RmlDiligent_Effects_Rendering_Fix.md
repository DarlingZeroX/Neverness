# RmlDiligent Effects 渲染修复记录

**日期**：2026-06-08
**问题**：RmlUi effects 示例的 DropShadow、filter 合成等多个渲染 bug
**影响范围**：所有使用 `drop-shadow`、`contrast`、`grayscale` 等 CSS filter 的元素

---

## 一、问题现象

1. **DropShadow div 内容不显示**：`.box.dropshadow` 和 `.box.animate` 的 div 背景、文字、图片不显示，只有阴影可见
2. **DropShadow 阴影方向错误**：GL3 官方阴影往右下，Diligent 往右上
3. **DropShadow 污染其他元素**：`.box.dropshadow` 导致相邻 `.box.contrast` 变白
4. **多元素 filter 合成偏白**：`.box.grayscale` 白色深浅与官方不一致，比官方更白
5. **渲染残留**：去掉某个元素后整个页面出现渲染残留，像没清 RenderTarget

---

## 二、根因分析与修复

### 修复 1：DropShadow 缺少原始内容叠加

**根因**：GL3 的 DropShadow filter 在 blur 之后、swap 之前，会将原始内容（primary）用 alpha blend 叠加到阴影（secondary）上。Diligent 缺少这步。

GL3 流程：
```
1. 渲染阴影到 secondary（禁用 blend）
2. 如果有 blur，对 secondary 做 blur
3. 用 blend 将 primary（原始内容）绘制到 secondary 上  ← 关键！
4. Swap
```

Diligent 流程（修复前）：
```
1. 渲染阴影到 secondary
2. 如果有 blur，对 secondary 做 blur
3. 直接 Swap  ← 缺少原始内容叠加！
```

**修复**：在 `RenderFilters` 的 `FilterType::DropShadow` 分支中，blur 之后、swap 之前，添加 passthrough blend：

```cpp
// 与 GL3 一致：blur 之后，将原始内容（primary）用 alpha blend 叠加到阴影（secondary）上。
DrawFullscreenPassthrough(primary.SRV.RawPtr(), secondary.RTV.RawPtr(), Rml::BlendMode::Blend, false);
```

**修改文件**：`Private/RmlDiligentRenderInterface.cpp` — `RenderFilters` DropShadow 分支

---

### 修复 2：DropShadow UV 偏移 Y 分量方向错误

**根因**：GL3 和 Diligent 的 tex coord 空间 Y 方向相反。

GL3 的 `VerticallyFlipped` 将 scissor 翻转后计算 tex coord：
- UV.y=0 → 屏幕**顶部**，UV.y↑ → 屏幕**下移**
- 公式：`offset / Vector2f(-W, +H)`

Diligent 不翻转 scissor：
- UV.y=0 → 屏幕**底部**，UV.y↑ → 屏幕**上移**
- 公式：`offset / Vector2f(-W, -H)` ← Y 取反

| | UV.y=0 对应 | UV.y 增大 → | UV 偏移公式 |
|---|---|---|---|
| GL3 (VerticallyFlipped) | 屏幕顶部 | 屏幕下移 | `offset / (-W, +H)` |
| Diligent (不翻转) | 屏幕底部 | 屏幕上移 | `offset / (-W, -H)` |

**修复**：

```cpp
// GL3 的 tex coord 空间经过 VerticallyFlipped：UV.y=0 → 屏幕顶部，UV.y↑ → 屏幕下移。
// Diligent 的 tex coord 空间不翻转：UV.y=0 → 屏幕底部，UV.y↑ → 屏幕上移。
// 因此 Y 分量需要取反（-H）才能匹配 GL3 的偏移方向。
const Rml::Vector2f uv_offset = filter.offset / Rml::Vector2f(-static_cast<float>(w), -static_cast<float>(h));
```

**修改文件**：`Private/RmlDiligentRenderInterface.cpp` — `RenderFilters` DropShadow 分支

---

### 修复 3：DropShadow filter 未清空 secondary RT

**根因**：Diligent 的 `CompositeLayers` 最终合成时使用**全屏 scissor**（为了不裁切 shadow/blur 的 halo），而 GL3 使用**元素 scissor**。

| | GL3 | Diligent |
|---|---|---|
| 最终合成 scissor | 元素 scissor | 全屏 scissor |
| secondary 残留内容 | 不可见（被元素 scissor 裁掉） | 可见（全屏 scissor 写出） |

当 `.box.dropshadow` 在 `.box.contrast` 之后处理时，secondary buffer 残留了 contrast filter 链的内容。DropShadow 的 shadow render 只在元素 scissor 内写入，scissor 外的残留内容通过全屏 scissor 写入目标 layer，污染了 contrast 元素的显示。

**修复**：在 DropShadow filter 开始时清空 secondary RT：

```cpp
// 清空 secondary RT，防止残留上一个元素 filter 链的内容。
Diligent::OptimizedClearValue zeroClear{};
zeroClear.Color[0] = zeroClear.Color[1] = zeroClear.Color[2] = zeroClear.Color[3] = 0.0f;
Diligent::ITextureView* pClearRTV = secondary.RTV.RawPtr();
m_Context->SetRenderTargets(1, &pClearRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
m_Context->ClearRenderTarget(pClearRTV, &zeroClear, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
```

**修改文件**：`Private/RmlDiligentRenderInterface.cpp` — `RenderFilters` DropShadow 分支

---

### 修复 4：CompositeLayers 未清空 secondary RT

**根因**：修复 3 只解决了 DropShadow filter 的问题，但**所有 filter** 都有同样的问题。

当页面有多个带 filter 的元素时：
1. 元素 A 的 `CompositeLayers`：filter 链处理后，secondary 有原始内容
2. 元素 B 的 `CompositeLayers`：`BlitLayerToPostprocessPrimary` 只覆盖 primary，**secondary 残留元素 A 的内容**
3. 元素 B 的 filter 只在元素 scissor 内写 secondary，scissor 外残留
4. 最终合成用全屏 scissor → 残留内容被写入目标 layer → 双重叠加 → 偏白

**修复**：在 `CompositeLayers` 的 `BlitLayerToPostprocessPrimary` 之后、`RenderFilters` 之前清空 secondary RT：

```cpp
BlitLayerToPostprocessPrimary(source);

// 清空 secondary RT，防止上一个 CompositeLayers 的 filter 链残留内容。
// GL3 最终合成用元素 scissor（残留不可见），Diligent 用全屏 scissor（残留会被写出）。
{
    auto& secondary = m_LayerStack.GetPostprocessSecondary();
    if (auto* pRTV = secondary.RTV.RawPtr()) {
        Diligent::OptimizedClearValue zero{};
        zero.Color[0] = zero.Color[1] = zero.Color[2] = zero.Color[3] = 0.0f;
        m_Context->SetRenderTargets(1, &pRTV, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_Context->ClearRenderTarget(pRTV, &zero, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }
}

RenderFilters(filters);
```

**修改文件**：`Private/RmlDiligentRenderInterface.cpp` — `CompositeLayers`

---

### 修复 5：BeginFrame 未清空 postprocess buffers

**根因**：`RenderTargetPool` 复用纹理时不清空内容。当 layer 栈深度变化（如去掉某个元素），pool 中纹理的复用模式改变，某些 postprocess buffer 可能复用了之前作为其他用途的纹理，残留内容通过全屏 scissor 写入目标 layer。

GL3 每帧重建 FBO（内容天然清空），Diligent 复用 pool 纹理需要显式清空。

**修复**：在 `BeginFrame` 中清空 postprocess primary 和 secondary：

```cpp
// 清空 postprocess buffers，防止 pool 复用纹理时残留上一帧/上一用途的内容。
{
    auto& pp0 = m_LayerStack.GetPostprocessPrimary();
    auto& pp1 = m_LayerStack.GetPostprocessSecondary();
    Diligent::OptimizedClearValue zero{};
    zero.Color[0] = zero.Color[1] = zero.Color[2] = zero.Color[3] = 0.0f;
    if (auto* rtv0 = pp0.RTV.RawPtr()) {
        m_Context->SetRenderTargets(1, &rtv0, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_Context->ClearRenderTarget(rtv0, &zero, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }
    if (auto* rtv1 = pp1.RTV.RawPtr()) {
        m_Context->SetRenderTargets(1, &rtv1, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_Context->ClearRenderTarget(rtv1, &zero, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }
}
// 清空 postprocess buffers 后重新绑定根 layer 为渲染目标
BindTopLayer();
```

**修改文件**：`Private/RmlDiligentRenderInterface.cpp` — `BeginFrame`

---

### 修复 6：Sampler CLAMP 导致 repeat 纹理无效

**根因**：RmlUi 的 `DecoratorTiled::Tile::GenerateGeometry` 对 `repeat`/`repeat-x`/`repeat-y` 模式会生成 UV 超出 [0,1] 范围的几何体（`repeat_factor = surface_dimensions / tile_dimensions`，可达 5+）。Diligent 的 sampler 设置为 `TEXTURE_ADDRESS_CLAMP`，超出范围的 UV 被钳制到边缘像素，repeat 效果不生效。

**修复**：将 sampler 的 AddressU/V/W 从 `TEXTURE_ADDRESS_CLAMP` 改为 `TEXTURE_ADDRESS_WRAP`。

```cpp
// RmlUi 的 DecoratorTiled 对 repeat 模式生成 UV 超出 [0,1] 的几何体，需要 WRAP 模式。
// 标准 RmlUi 几何体 UV 在 [0,1] 范围内，WRAP 模式无副作用。
samplerDesc.AddressU = Diligent::TEXTURE_ADDRESS_WRAP;
samplerDesc.AddressV = Diligent::TEXTURE_ADDRESS_WRAP;
samplerDesc.AddressW = Diligent::TEXTURE_ADDRESS_WRAP;
```

**修改文件**：`Private/RmlDiligentRenderInterface.cpp` — sampler 创建

---

## 三、关键教训

### 1. GL3 全屏 scissor 与 Diligent 全屏 scissor 的语义差异
- GL3 的 `CompositeLayers` **不设 scissor**，使用 filter 链留下的元素 scissor
- Diligent 的 `CompositeLayers` 设**全屏 scissor**（为了不裁切 shadow/blur halo）
- 后果：GL3 的元素 scissor 自然裁掉了 postprocess buffer 的残留内容，Diligent 的全屏 scissor 会把残留写出

### 2. Postprocess buffer 必须在每次使用前清空
- `BlitLayerToPostprocessPrimary` 只覆盖 primary，不覆盖 secondary
- secondary 可能残留上一次 `CompositeLayers` 的 filter 链输出
- 每个 filter 只在 scissor 区域内写 secondary，scissor 外必须是透明（0）
- 修复：`CompositeLayers` 入口清空 secondary

### 3. RenderTargetPool 复用纹理不清空
- Pool 的 `Acquire` 方法返回复用纹理时不清空内容
- 新建纹理内容未定义
- 每帧必须显式清空所有会使用的 RT
- 修复：`BeginFrame` 清空 postprocess buffers

### 4. UV 偏移方向取决于 tex coord 空间
- GL3 用 `VerticallyFlipped` scissor 计算 tex coord → UV.y↑ = 屏幕下移
- Diligent 不翻转 → UV.y↑ = 屏幕上移
- 同一公式 `offset / (-W, H)` 在两个系统中含义不同
- 修复：Diligent 用 `offset / (-W, -H)` 取反 Y

---

## 四、修改文件清单

| 文件 | 修改内容 |
|------|---------|
| `Private/RmlDiligentRenderInterface.cpp` | DropShadow passthrough blend、UV Y 取反、DropShadow secondary clear、CompositeLayers secondary clear、BeginFrame postprocess clear、Sampler CLAMP→WRAP |

---

## 五、验证

- ✅ `.box.dropshadow`：div 内容和阴影都正确显示
- ✅ `.box.animate`：动画 drop-shadow + opacity + sepia 链正确
- ✅ `.box.contrast`：不被 dropshadow 污染，白色深浅正确
- ✅ `.box.grayscale`：白色深浅与 GL3 一致
- ✅ 去掉任意元素后无渲染残留
- ✅ demo 示例 repeat/repeat-x/repeat-y 纹理平铺正确
