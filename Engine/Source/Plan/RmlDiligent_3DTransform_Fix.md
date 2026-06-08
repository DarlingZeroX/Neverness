# RmlDiligent 3D Transform 渲染修复完整记录

**日期**：2026-06-07
**问题**：RmlUi + Diligent Engine 后端的 3D Transform 渲染存在多个严重 bug
**影响范围**：所有使用 `rotate3d()`、`perspective()`、`translateZ()` 的元素

---

## 一、问题现象

1. **box-shadow 残影穿透**：带 blur 的 box-shadow 在 3D 旋转时，残影穿透到 div 内部
2. **3D 旋转裁剪**：整个窗口在 `rotate3d()` 时只显示左半边或右半边
3. **Cube 面错位**：transform sample 的立方体各面到处乱飞
4. **文字截断**：3D 旋转元素内的文字被裁剪
5. **窗口内容溢出**：3D 旋转后窗口内容超出底部不裁剪
6. **box-shadow 模糊程度淡**：blur 效果比官方 GL3/DX12 后端淡很多

---

## 二、根因分析与修复

### 修复 1：矩阵乘法顺序（最关键）

**根因**：HLSL shader 中 `row_major float4x4` + `mul(v, M)` 与 RmlUi 的 `ColumnMajorStorage` 不匹配。

RmlUi 的 `Matrix4f` 使用 `ColumnMajorStorage`。`SetRows` 通过 `StrideVector` 写入，数据实际按列存储：
```
vectors[0] = Column 0 = (2/(r-l), 0, 0, -(r+l)/(r-l))
vectors[1] = Column 1 = (0, 2/(t-b), 0, -(t+b)/(t-b))
...
data()[12..14] = Translation
```

这恰好是 HLSL `float4x4`（默认列主序）的布局。

**错误做法**（修复前）：
```hlsl
cbuffer ConstantBuffer {
    row_major float4x4 m_transform;  // 声明行主序
    float2 m_translate;
};
float4 resPos = mul(float4(pos, 0, 1), m_transform);  // v * M
```
列主序数据被行主序读取 → 矩阵被隐式转置 → `v * M^T` → 3D 旋转产生错切。

**正确做法**（与 DX12 参考实现一致）：
```hlsl
cbuffer ConstantBuffer {
    float4x4 m_transform;   // 默认列主序
    float2 m_translate;
};
float4 resPos = mul(m_transform, float4(pos, 0, 1));  // M * v
```

**修改文件**：`Shaders/RmlDiligent_Shaders.h` — 所有 shader 移除 `row_major`，改用 `mul(M, v)`

---

### 修复 2：投影与 CSS Transform 分离 + 平移顺序修正

**根因**：之前 `m_Transform = m_Projection * (*transform)`，投影和 CSS transform 合为一体。shader 中 `pos + translate` 在 transform 之前加，导致：

```
GL3:    Projection * Transform * (pos + translate)   ← 正确
Diligent: (Projection * Transform) * (pos + translate) ← 平移在错误空间
```

RmlUi 的 transform 矩阵已包含元素位置（`T(origin) * Rotate * T(-origin) * T(pos)`），如果在 transform 前再加 `translate`，会导致**双重平移** → 窗口偏移翻倍 → 只显示半边。

**修复**：投影矩阵分离到独立的 `ProjectionBuffer`，shader 中匹配 GL3 顺序：

```hlsl
cbuffer ConstantBuffer {
    float4x4 m_transform;   // CSS transform（不含投影）
    float2 m_translate;     // 几何平移
};
cbuffer ProjectionBuffer {
    float4x4 m_projection;  // 正交投影
};

// 与 GL3 一致：Projection * (Transform * (pos + translate))
float2 translatedPos = IN.position + m_translate;
float4 resPos = mul(m_projection, mul(m_transform, float4(translatedPos, 0, 1)));
```

**修改文件**：
- `Shaders/RmlDiligent_Shaders.h` — VS_Main 添加 ProjectionBuffer
- `Private/RmlDiligentRenderInterface.cpp` — `SetTransform` 存储纯 CSS transform，`SetProjectionMatrix` 上传投影到 ProjectionBuffer
- `Public/RmlDiligentRenderInterface.h` — 添加 `m_ProjectionCB` 成员
- 所有 SRB 创建代码绑定 ProjectionBuffer（包括 Gradient/Creation 及其 StencilEqual 变体）

---

### 修复 3：D3D12 NDC Z 范围（裁剪根因）

**根因**：RmlUi 的 `ProjectOrtho` 输出 Z ∈ [-1, 1]（OpenGL 约定），但 Diligent/D3D12 使用 Z ∈ [0, 1]。

| | Z 范围 | 裁剪条件 |
|---|---|---|
| OpenGL | [-1, 1] | z < -1 或 z > 1 |
| D3D12 | [0, 1] | z < 0 或 z > 1 |

3D 旋转后 Z 变负 → D3D12 GPU 裁剪（z < 0）→ 只显示半边窗口。

**修复**：手动构建 D3D12 约定的正交投影矩阵：

```cpp
// D3D12 正交投影：Z 映射到 [0, 1]
d[0]  = 2.f / (r - l);        // X 缩放
d[5]  = 2.f / (t - b);        // Y 缩放
d[10] = 1.f / (f - n);        // Z 缩放 — 1/(f-n)，不是 2/(f-n)
d[12] = -(r + l) / (r - l);   // X 平移
d[13] = -(t + b) / (t - b);   // Y 平移
d[14] = -n / (f - n);         // Z 平移 — -n/(f-n)，不是 -(f+n)/(f-n)
d[15] = 1.f;
```

**修改文件**：`Private/RmlDiligentRenderInterface.cpp` — `SetProjectionMatrix`

---

### 修复 4：isTransformed 检测逻辑

**根因**：`isTransformed` 之前用 `memcmp(m_Transform, m_Projection)` 检测。分离投影后 `m_Transform` 变成纯 CSS transform，与 `m_Projection`（正交投影）永远不同 → 所有元素被误判为 transformed → scissor 被错误禁用。

**修复**：改为与 `Identity` 比较：
```cpp
const Rml::Matrix4f& identity = Rml::Matrix4f::Identity();
const bool isTransformed = memcmp(m_Transform.data(), identity.data(), sizeof(float) * 16) != 0;
```

**修改文件**：`Private/RmlDiligentRenderInterface.cpp` — `RenderGeometry` 和 `DrawIndexedGeometry`

---

### 修复 5：3D Transform Scissor 禁用

**根因**：`DrawIndexedGeometry` 内部在绘制后恢复 `m_ScissorRegionRml` 到 GPU scissor。如果 `m_ScissorRegionRml` 是元素 2D 边界，恢复后的 scissor 会裁剪 3D 变换后的顶点。

**修复**：3D 变换时临时把 `m_ScissorRegionRml` 设为全屏，绘制后恢复：
```cpp
if (restoreGeometryScissor) {
    savedScissorRegion = m_ScissorRegionRml;
    m_ScissorRegionRml = Rml::Rectanglei::FromCorners({0,0}, {sw, sh});
    EnableScissorRegion(false);
}
// ... draw ...
if (restoreGeometryScissor) {
    m_ScissorRegionRml = savedScissorRegion;
    EnableScissorRegion(true);
}
```

---

### 修复 6：CompositeLayers Stencil 测试

**根因**：box-shadow blur composite 使用 `m_PSO_Passthrough`（StencilMode::Off），忽略 clip mask，模糊阴影穿透到元素内部。

**修复**：当 clip mask 激活时使用 `m_PSO_Passthrough_StencilEqual`：
```cpp
if (m_ClipMaskEnabled && m_UseStencilEqual && !composite_pso) {
    composite_pso = m_PSO_Passthrough_StencilEqual;
    m_Context->SetStencilRef(m_StencilTestValue);
}
```

**修改文件**：`Private/RmlDiligentRenderInterface.cpp` — `CompositeLayers`

---

### 修复 7：Blur 算法重构（box-shadow 模糊程度）

**根因**：GL3/DX12 后端的 `RenderBlur` 先 downscale 到小分辨率 RT，然后在小 RT 上做 blur。每个 blur tap 在小分辨率上覆盖更多像素，等效于更大的模糊范围。Diligent 后端始终在全分辨率上做 blur，同样的 sigma 值覆盖范围小很多 → 模糊效果淡。

**修复**：重构 `RenderBlur`，使用小分辨率 RT 匹配 GL3 算法：

```cpp
// 1. 计算 downscale 后的分辨率（与 GL3 一致的 halving）
int small_w = fb_w, small_h = fb_h;
for (int i = 0; i < pass_level; ++i) {
    small_w = Max(1, (small_w + 1) / 2);
    small_h = Max(1, (small_h + 1) / 2);
}

// 2. 分配小分辨率 RT
auto rtSmallDst = AcquireColor(small_w, small_h);
auto rtSmallTmp = AcquireColor(small_w, small_h);

// 3. Downscale：全分辨率 → 小分辨率（双线性插值）
DrawFullscreenPassthrough(source → rtSmallDst);

// 4. Two-pass Gaussian blur at reduced resolution
//    sigma 和 texel offset 与 GL3 完全一致
UploadBlurCB(blur_sigma, {0, 1/small_h}, ...);  // vertical
DrawBlurPass(rtSmallDst → rtSmallTmp);
UploadBlurCB(blur_sigma, {1/small_w, 0}, ...);  // horizontal
DrawBlurPass(rtSmallTmp → rtSmallDst);

// 5. Upscale：小分辨率 → 全分辨率
DrawFullscreenPassthrough(rtSmallDst → source_destination);
```

**关键点**：
- sigma 不变（与 GL3 一致的 `SigmaToParameters` 计算）
- texel offset 基于小分辨率 RT 尺寸
- 每个 blur tap 在小分辨率上覆盖更多像素，等效于 GL3 的 downscale+blur
- 小 RT 从 `RenderTargetPool` 分配，用完自动回收

**修改文件**：`Private/RmlDiligentRenderInterface.cpp` — `RenderBlur`

---

### 修复 8：ProjectionBuffer 未绑定到 StencilEqual PSO

**根因**：Gradient_StencilEqual 和 Creation_StencilEqual PSO 的变量声明中缺少 `ProjectionBuffer`，导致运行时报错 "No resource is bound to variable 'ProjectionBuffer'"。

**修复**：所有使用 VS_Main 的 PSO 都必须在变量声明中包含 ProjectionBuffer：
```cpp
Diligent::ShaderResourceVariableDesc shaderCBVarsSE[] = {
    {SHADER_TYPE_VERTEX, "ConstantBuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
    {SHADER_TYPE_VERTEX, "ProjectionBuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
    {SHADER_TYPE_PIXEL, "SharedConstantBuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
};
```

**修改文件**：`Private/RmlDiligentRenderInterface.cpp` — Gradient/Creation StencilEqual PSO 创建

---

## 三、关键教训

### 1. 矩阵存储布局必须与 shader 声明一致
- RmlUi `ColumnMajorStorage` → 数据按列存储
- HLSL `float4x4`（默认）= 列主序 → 直接匹配
- HLSL `row_major float4x4` = 行主序 → 需要转置
- **DX12 参考实现是最好的对照**

### 2. NDC Z 范围因图形 API 而异
- OpenGL/WebGL：Z ∈ [-1, 1]
- D3D11/D3D12/Vulkan/Metal：Z ∈ [0, 1]
- RmlUi 的 `ProjectOrtho` 使用 OpenGL 约定，Diligent 后端必须手动修正

### 3. 平移必须在正确的坐标空间
- GL3：`Transform * (pos + translate)` — 平移在本地空间
- 如果投影和 CSS transform 合并，平移必须在 CSS transform 之后、投影之前
- 否则会导致双重平移

### 4. RmlUi 的 transform 矩阵已包含元素位置
- `T(origin) * Rotate * T(-origin) * T(screen_pos)`
- shader 中的 `translate` 是几何偏移，不是元素位置
- 不能在 transform 前重复添加

### 5. Blur 必须在降分辨率 RT 上工作
- GL3/DX12 的 downscale 不只是优化，它改变了 blur 的有效覆盖范围
- 同样的 sigma 在全分辨率上效果淡很多
- 必须分配小分辨率 RT，在小 RT 上做 blur，再 upscale 回来

### 6. 所有共享 VS_Main 的 PSO 必须同步 CB 布局
- VS_Main 被 Color/Texture/Gradient/Creation PSO 共享
- 改变 VS_Main 的 CB 布局后，所有 PSO 的变量声明和 SRB 绑定都必须同步更新
- 包括 StencilEqual 变体

---

## 四、验证矩阵正确性的方法

在 `SetTransform` 中打印关键值：
```
[MVP] P[0], P[5]  — 投影缩放（应 ≈ 2/width, -2/height）
[MVP] P[12], P[13] — 投影平移（应 ≈ -1, 1）
[MVP] T[3,7,11,15] — w 行（正交应为 0,0,0,1）
[W]   T[15]         — w 分量（应 = 1.0 for 正交）
```

在 shader 中强制 `resPos.w = 1.0` 测试：如果窗口变完整，确认是 w 分量问题。

---

## 五、修改文件清单

| 文件 | 修改内容 |
|------|---------|
| `Shaders/RmlDiligent_Shaders.h` | VS_Main：移除 `row_major`，改用 `mul(M, v)`，添加 ProjectionBuffer，修正平移顺序 |
| `Private/RmlDiligentRenderInterface.cpp` | SetTransform/ProjectionMatrix/CB 上传/SRB 绑定/isTransformed 检测/scissor 禁用/clip mask 清除/composite stencil/blur 重构/StencilEqual PSO 修复 |
| `Public/RmlDiligentRenderInterface.h` | 添加 `m_ProjectionCB` 成员 |
| `Renderer/RmlDiligentRenderTargetPool.cpp` | 无修改（复用现有 AcquireColor 接口） |
