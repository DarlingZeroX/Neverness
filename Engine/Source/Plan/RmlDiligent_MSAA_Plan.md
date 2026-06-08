# RmlDiligent MSAA 抗锯齿实施计划

**日期**：2026-06-08
**目标**：为 Diligent 后端添加 2x MSAA 抗锯齿，与 GL3/DX12 官方后端效果对齐
**范围**：仅 RmlDiligent 模块，不修改 ThirdParty

---

## 一、现状分析

### GL3 后端 MSAA 架构
- Layer RT：`glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, ...)` — 2x MSAA
- Depth/Stencil：同样 2x MSAA
- Postprocess RT：1x（非 MSAA）
- SwapChain：1x
- Resolve：`glBlitFramebuffer` 从 MSAA layer → 非 MSAA postprocess（隐式 resolve）

### DX12 后端 MSAA 架构
- Layer RT：`SampleDesc.Count = msaa_sample_count`
- Depth/Stencil：同样 MSAA
- Postprocess RT：1x
- SwapChain：1x
- Resolve：`ResolveSubresource` 显式 resolve

### Diligent 后端现状
- 所有 RT（Layer/Postprocess/DepthStencil）采样数 = 1
- SwapChain 采样数 = 1
- 无 MSAA，无 Resolve

---

## 二、架构设计

### 核心原则
1. **MSAA 只用于 Layer 离屏 RT**（与 GL3/DX12 一致）
2. **Postprocess RT 保持 1x**（filter 链不需要 MSAA）
3. **SwapChain 保持 1x**（最终输出不需要 MSAA）
4. **Resolve 在 BlitLayerToPostprocessPrimary 中完成**（layer → postprocess）

### 纹理采样数分配

| 纹理类型 | 采样数 | 说明 |
|----------|--------|------|
| Layer Color RT | 2 | MSAA 渲染目标 |
| Layer DepthStencil | 2 | 与 Layer Color 匹配 |
| Postprocess Primary/Secondary/Tertiary | 1 | filter 链使用 |
| BlendMask | 1 | mask 使用 |
| SwapChain | 1 | 最终输出 |

### MSAA Resolve 方案

Diligent Engine 提供专门的 `ResolveTextureSubresource` API（等同于 D3D12 的 `ResolveSubresource`）：

```cpp
Diligent::ResolveTextureSubresourceAttribs resolveAttribs{};
resolveAttribs.SrcMipLevel = 0;
resolveAttribs.SrcSlice = 0;
resolveAttribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
resolveAttribs.DstMipLevel = 0;
resolveAttribs.DstSlice = 0;
resolveAttribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
resolveAttribs.Format = Diligent::TEX_FORMAT_UNKNOWN;  // 源和目标格式相同
m_Context->ResolveTextureSubresource(msaaTexture, nonMsaaTexture, resolveAttribs);
```

- 源纹理必须是 MSAA（SampleCount > 1）
- 目标纹理必须是非 MSAA（SampleCount = 1）
- 格式必须匹配（或兼容的 typeless 格式）
- 这是硬件级别的 resolve，性能最优

---

## 三、实施步骤

### Phase 1：RenderTargetPool 支持 MSAA 纹理

**文件**：`Renderer/RmlDiligentRenderTargetPool.h`, `Renderer/RmlDiligentRenderTargetPool.cpp`

修改：
1. `CreateNew` 添加 `int samples` 参数
2. `Acquire` 添加 `int samples` 参数，匹配时检查采样数
3. 新增 `AcquireColorMSAA` 辅助函数（samples=2）
4. `AcquireDepthStencilMSAA` 辅助函数（samples=2）
5. `PooledRenderTarget` 添加 `int samples` 成员

```cpp
// 新增
static PooledRTHandle AcquireColorMSAA(RenderTargetPool& pool, int width, int height, int samples = 2);
static PooledRTHandle AcquireDepthStencilMSAA(RenderTargetPool& pool, int width, int height, int samples = 2);
```

### Phase 2：RenderLayerStack 使用 MSAA Layer RT

**文件**：`Renderer/RmlDiligentLayerStack.h`, `Renderer/RmlDiligentLayerStack.cpp`

修改：
1. `PushLayer` 使用 `AcquireColorMSAA` 创建 MSAA layer RT
2. 共享 DepthStencil 使用 `AcquireDepthStencilMSAA` 创建 MSAA
3. Postprocess RT 保持 1x（不变）
4. 添加 `GetLayerSamples()` 查询当前 layer 采样数

```cpp
// PushLayer 修改
if (m_LayerCount == static_cast<int>(m_LayerColors.size())) {
    m_LayerColors.push_back(RenderTargetPool::AcquireColorMSAA(*m_Pool, m_Width, m_Height, m_MsaaSamples));
}

// 共享 DepthStencil 修改
if (!m_SharedDepth) {
    m_SharedDepth = RenderTargetPool::AcquireDepthStencilMSAA(*m_Pool, m_Width, m_Height, m_MsaaSamples);
}
```

### Phase 3：RenderPass 支持 MSAA

**文件**：`Private/RmlDiligentRenderInterface.cpp`

修改 `CreateRenderPass`：
1. Color attachment 设置 `SampleCount = 2`
2. DepthStencil attachment 设置 `SampleCount = 2`

```cpp
void RmlDiligentRenderInterface::CreateRenderPass(int msaa_samples)
{
    Diligent::RenderPassAttachmentDesc attachments[2]{};

    attachments[0].Format = m_SwapChain->GetDesc().ColorBufferFormat;
    attachments[0].LoadOp = Diligent::ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].StoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;
    attachments[0].SampleCount = static_cast<Diligent::Uint8>(msaa_samples);  // ← 新增

    attachments[1].Format = Diligent::TEX_FORMAT_D24_UNORM_S8_UINT;
    attachments[1].LoadOp = Diligent::ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].StoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;
    attachments[1].StencilLoadOp = Diligent::ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].StencilStoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;
    attachments[1].SampleCount = static_cast<Diligent::Uint8>(msaa_samples);  // ← 新增
    // ...
}
```

### Phase 4：PSO 支持 MSAA

**文件**：`Private/RmlDiligentRenderInterface.cpp`

修改 `setupCommonPipeline`：
1. 设置 `SmplDesc.Count = 2`（MSAA 采样数）

```cpp
auto setupCommonPipeline = [&](Diligent::GraphicsPipelineStateCreateInfo& psoCI, ...) {
    // ... 现有代码 ...
    psoCI.GraphicsPipeline.SmplDesc.Count = m_MsaaSamples;  // ← 新增
    // ...
};
```

**注意**：Postprocess 专用 PSO（Blur、DropShadow、ColorMatrix、BlendMask、Passthrough）使用 1x 采样数，因为它们操作的是非 MSAA 的 postprocess RT。

### Phase 5：BlitLayerToPostprocessPrimary 支持 MSAA Resolve

**文件**：`Private/RmlDiligentRenderInterface.cpp`

修改 `BlitLayerToPostprocessPrimary`：
1. 如果 layer 是 MSAA，使用 `ResolveTextureSubresource` 进行硬件 resolve
2. 如果 layer 是非 MSAA，使用 `CopyTexture` 直接复制

```cpp
void RmlDiligentRenderInterface::BlitLayerToPostprocessPrimary(Rml::LayerHandle layer_handle)
{
    const auto& source = m_LayerStack.GetLayer(layer_handle);
    auto& destination = m_LayerStack.GetPostprocessPrimary();
    if (!source.texture || !destination.texture) {
        return;
    }

    // 设置全屏 scissor
    const bool previous_scissor_enabled = m_ScissorEnabled;
    m_ScissorEnabled = false;
    if (m_Context && m_SwapChain) {
        const int w = static_cast<int>(m_SwapChain->GetDesc().Width);
        const int h = static_cast<int>(m_SwapChain->GetDesc().Height);
        Diligent::Rect fullScissor{};
        fullScissor.left = 0;
        fullScissor.top = 0;
        fullScissor.right = w;
        fullScissor.bottom = h;
        m_Context->SetScissorRects(1, &fullScissor, static_cast<Diligent::Uint32>(w), static_cast<Diligent::Uint32>(h));
    }

    UnbindRenderTargets();

    if (source.samples > 1) {
        // MSAA Resolve：使用 Diligent 的 ResolveTextureSubresource（等同 D3D12 ResolveSubresource）
        Diligent::ResolveTextureSubresourceAttribs resolveAttribs{};
        resolveAttribs.SrcMipLevel = 0;
        resolveAttribs.SrcSlice = 0;
        resolveAttribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        resolveAttribs.DstMipLevel = 0;
        resolveAttribs.DstSlice = 0;
        resolveAttribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        resolveAttribs.Format = Diligent::TEX_FORMAT_UNKNOWN;
        m_Context->ResolveTextureSubresource(source.texture, destination.texture, resolveAttribs);
    } else {
        // 非 MSAA：直接复制
        Diligent::CopyTextureAttribs copyAttribs;
        copyAttribs.pSrcTexture = source.texture;
        copyAttribs.pDstTexture = destination.texture;
        copyAttribs.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        copyAttribs.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        m_Context->CopyTexture(copyAttribs);
    }

    m_ScissorEnabled = previous_scissor_enabled;
    if (previous_scissor_enabled && m_ScissorRegionRml.Valid()) {
        SetScissorRml(m_ScissorRegionRml, false);
    }
}
```

### Phase 6：Postprocess PSO 保持 1x 采样数

**文件**：`Private/RmlDiligentRenderInterface.cpp`

需要为 postprocess 专用 PSO 创建独立的 pipeline setup，不使用 MSAA：

```cpp
// Postprocess PSO 使用 1x 采样数
auto setupPostprocessPipeline = [&](Diligent::GraphicsPipelineStateCreateInfo& psoCI, ...) {
    // ... 与 setupCommonPipeline 相同，但 SmplDesc.Count = 1
    psoCI.GraphicsPipeline.SmplDesc.Count = 1;  // ← 非 MSAA
};
```

需要修改以下 PSO 的创建：
- `m_PSO_Passthrough`
- `m_PSO_PassthroughPresent`
- `m_PSO_Passthrough_StencilEqual`
- `m_PSO_PassthroughOpacity`
- `m_PSO_PassthroughReplace`
- `m_PSO_Blur`
- `m_PSO_DropShadow`
- `m_PSO_ColorMatrix`
- `m_PSO_BlendMask`

### Phase 7：BeginFrame 清空 MSAA RT

**文件**：`Private/RmlDiligentRenderInterface.cpp`

`BeginFrame` 中清空根 layer RTV 时，需要处理 MSAA 纹理。`ClearRenderTarget` 应该能直接处理 MSAA RTV。

### Phase 8：验证与调试

1. 编译运行 effects 示例，对比 GL3 效果
2. 检查边缘锯齿是否改善
3. 检查 stencil/clip mask 在 MSAA 下是否正常
4. 检查 filter 链（blur/drop-shadow/color-matrix）是否正常
5. 性能对比（MSAA 2x 的开销）

---

## 四、风险与注意事项

### 1. ResolveTextureSubresource 支持
- Diligent 提供专门的 `ResolveTextureSubresource` API（等同 D3D12 `ResolveSubresource`）
- 源纹理必须 MSAA，目标纹理必须非 MSAA，格式必须匹配
- 这是硬件级别的 resolve，所有 Diligent 后端（D3D12/Vulkan/GL）都支持
- 无需 shader-based resolve 回退方案

### 2. Postprocess PSO 必须是非 MSAA
- Postprocess RT 是 1x，PSO 的 `SmplDesc.Count` 必须是 1
- 如果 PSO 的采样数与 RT 的采样数不匹配，D3D12 会报错
- 需要区分 Layer PSO（MSAA）和 Postprocess PSO（1x）

### 3. DepthStencil 共享
- 所有 Layer 共享同一个 DepthStencil（当前实现）
- DepthStencil 必须是 MSAA（与 Layer RT 匹配）
- Postprocess 不使用 DepthStencil（当前实现正确）

### 4. Stencil 操作在 MSAA 下的行为
- Stencil 写入/测试在 MSAA RT 上逐采样点操作
- 应该与非 MSAA 行为一致，但需要验证

### 5. 性能影响
- 2x MSAA 的显存开销约为 2x（Layer RT + DepthStencil）
- 渲染开销约为 10-20%（取决于 GPU）
- Postprocess 不受影响（保持 1x）

---

## 五、修改文件清单

| 文件 | 修改内容 |
|------|---------|
| `Renderer/RmlDiligentRenderTargetPool.h` | 添加 samples 参数、MSAA 辅助函数、samples 成员 |
| `Renderer/RmlDiligentRenderTargetPool.cpp` | CreateNew/Acquire 支持 samples、MSAA 辅助函数实现 |
| `Renderer/RmlDiligentLayerStack.h` | 添加 m_MsaaSamples 成员、GetLayerSamples() |
| `Renderer/RmlDiligentLayerStack.cpp` | PushLayer 使用 MSAA RT、共享 DepthStencil 使用 MSAA |
| `Private/RmlDiligentRenderInterface.cpp` | CreateRenderPass MSAA、setupCommonPipeline MSAA、setupPostprocessPipeline、BlitLayerToPostprocessPrimary resolve、BeginFrame MSAA clear |
| `Public/RmlDiligentRenderInterface.h` | 添加 m_MsaaSamples 成员 |

---

## 六、实施顺序

1. **Phase 1**：RenderTargetPool 支持 samples 参数（基础设施）
2. **Phase 2**：LayerStack 使用 MSAA RT（核心改动）
3. **Phase 3**：RenderPass 支持 MSAA（管线配置）
4. **Phase 4**：PSO 区分 MSAA/非 MSAA（管线配置）
5. **Phase 5**：BlitLayerToPostprocessPrimary resolve（关键逻辑）
6. **Phase 6**：Postprocess PSO 保持 1x（修正）
7. **Phase 7**：BeginFrame 清空 MSAA RT（完整性）
8. **Phase 8**：验证与调试

建议按 Phase 顺序逐步实施，每完成一个 Phase 编译测试一次。
