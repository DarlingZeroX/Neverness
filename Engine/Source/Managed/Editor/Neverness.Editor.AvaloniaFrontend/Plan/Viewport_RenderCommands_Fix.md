# Viewport RenderCommands 渲染修复

**日期**: 2026-06-19
**问题**: Viewport 黑屏，Sprite 不显示

---

## 修复清单

### 1. Diligent ImmutableSampler 绑定错误

**文件**: `NNRuntimeRenderer2D/Source/Renderer2D.cpp` + `Include/Renderer2D/BuiltinShaders.h`

**错误信息**:
```
Diligent Engine: ERROR: Shader 'SpritePS' uses combined texture samplers,
but sampler 'u_Sampler' is not assigned to any texture
```

**根因**: Diligent 使用 combined texture samplers 模式时，HLSL 中的 `SamplerState` 必须命名为 `{TextureName}_sampler`，且 `ImmutableSamplerDesc` 的 `SamplerOrTextureName` 必须填 **texture 变量名**。

**修复**:
- HLSL shader: `SamplerState u_Sampler` → `SamplerState u_Texture_sampler`
- `ImmutableSamplerDesc`: `"u_Sampler"` → `"u_Texture"`

**Diligent 文档引用**:
> `SamplerOrTextureName` — The name of the sampler itself or the name of the texture variable that this immutable sampler is assigned to if combined texture samplers are used.

---

### 2. ViewProjection 矩阵未计算

**文件**: `NNRuntimeEngineServices/Private/ViewportSurface/ViewportSurfaceRuntimeApi.cpp` — `ConvertToCameraData()`

**错误**: `ViewProjectionMatrix` 被设为 `defaultData`（零矩阵/单位矩阵），Renderer2D 直接 memcpy 使用，不自行计算。

**修复**: 在 `ConvertToCameraData` 中手算 `VP = P * V`（列主序矩阵乘法），与 `SceneRenderer::FindMainCamera` 一致。

---

### 3. 矩阵转置错误（核心问题）

**文件**: `Neverness.Editor.Scene/Private/Service/ViewportServiceImpl.cs` — `CollectRenderCommands()`

**症状**: Sprite 变形为梯形，Position 调整不是位移而是透视变形。

**根因**: C# 端对 View/Projection/WorldMatrix 做了 `Matrix4x4.Transpose()`，但 System.Numerics 行主序的内存布局 **恰好等于** HLSL 列主序的布局：

```
System.Numerics 行主序: [M11, M12, M13, M14, M21, M22, M23, M24, M31, M32, M33, M34, M41, M42, M43, M44]
HLSL 列主序读取:        Col0=[M11,M12,M13,M14], Col1=[M21,M22,M23,M24], ..., Col3=[M41,M42,M43,M44]
```

平移 `M41/M42/M43` 自动成为 `M[0][3]/M[1][3]/M[2][3]`（`M * v` 的正确平移位置）。

手动转置后，平移从 `M41` 跑到 `M14`，HLSL 读为 `M[3][0]`——平移变成了 **透视除法分量**：

```
// 转置后 HLSL 矩阵:
result[3] = M[3][0]*x + M[3][1]*y + M[3][2]*z + M[3][3]*w  // M[3][0]=Tx!
// w = Tx*x + 1 → NDC_x = x / (Tx*x + 1) → 梯形变形
```

**修复**: 去掉所有 `Matrix4x4.Transpose()` 调用，直接传原始矩阵。

---

## 关键教训

1. **System.Numerics ↔ HLSL 矩阵不需要转置**: 行主序内存布局 = 列主序内存布局。这是 .NET 和 DirectX 互操作的标准做法。

2. **不要混淆数学约定和内存布局**: 数学上行主序和列主序是转置关系，但内存中同一矩阵的字节排列可以相同（取决于怎么定义"行"和"列"）。

3. **Diligent combined texture samplers 命名约定**: sampler 必须命名为 `{TextureName}_sampler`，`ImmutableSamplerDesc` 填 texture 名。

4. **C++ 和 C# 端的矩阵约定必须一致**: GLM 列主序 `P * V` = System.Numerics 行主序 `P * V`，因为底层字节相同。
