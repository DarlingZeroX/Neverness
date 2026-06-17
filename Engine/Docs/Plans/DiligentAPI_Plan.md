# NNDiligentAPI 子表实现计划

## 概述

在 `NNNativeEngineAPI` 中新增 `NNDiligentAPI` 子表，暴露底层 Diligent 设备指针供外部模块直接使用。

## 需求

4 个 API：
1. `GetPrimaryDevice()` → `::Diligent::IRenderDevice*`
2. `GetPrimaryContext()` → `::Diligent::IDeviceContext*`
3. `GetPrimarySwapChain()` → `::Diligent::ISwapChain*`
4. `CreateViewportSurfaceWithSwapChain()` → 创建 Surface 并返回其 `::Diligent::ISwapChain*`

## 架构分析

### 现有数据流

```
VGWindow (主窗口)
  └─ GetDevice() → INNRenderDevice*
       └─ static_cast<NNDiligentDevice*>
            ├─ GetDiligentDevice()   → IRenderDevice*
            ├─ GetDiligentContext()  → IDeviceContext*
            └─ GetDiligentSwapChain() → ISwapChain*
```

### 依赖关系

- `NNRuntimeEngineServices` 链接 `NNRuntimeDiligent`（已有依赖）
- `NNDiligentDevice` 提供底层 Diligent 指针访问
- `WindowRegistry::GetPrimaryHandle()` 获取主窗口句柄

## 修改文件清单

### 1. 新建 `DiligentAPI.h`

**路径**: `Engine/Source/Runtime/NNNativeEngineAPI/Include/DiligentAPI.h`

```c
#pragma once

/**
 * @file DiligentAPI.h
 * @brief Diligent 底层设备指针暴露 API。
 *
 * 职责：
 * - 获取主窗口的 Diligent IRenderDevice / IDeviceContext / ISwapChain
 * - 创建 ViewportSurface 并直接返回其 ISwapChain*
 *
 * 使用场景：
 * - C# 端通过 NativeAOT 直接调用 Diligent API
 * - 外部渲染模块需要底层设备指针
 *
 * v28 新增子表。
 */

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NNDiligentAPI
{
    /**
     * @brief 获取主窗口的 Diligent IRenderDevice*。
     * @return 设备指针（nullptr = 主窗口未初始化或非 Diligent 后端）
     */
    void* (NN_ENGINE_ABI_STDCALL *GetPrimaryDevice)(void);

    /**
     * @brief 获取主窗口的 Diligent IDeviceContext*。
     * @return 上下文指针（nullptr = 主窗口未初始化或非 Diligent 后端）
     */
    void* (NN_ENGINE_ABI_STDCALL *GetPrimaryContext)(void);

    /**
     * @brief 获取主窗口的 Diligent ISwapChain*。
     * @return SwapChain 指针（nullptr = 主窗口未初始化或非 Diligent 后端）
     */
    void* (NN_ENGINE_ABI_STDCALL *GetPrimarySwapChain)(void);

    /**
     * @brief 创建 ViewportSurface 并返回其 ISwapChain*。
     *
     * 与 ViewportSurfaceAPI.CreateSurface 的区别：
     * - CreateSurface 返回 surfaceId（需通过其他 API 获取 SwapChain）
     * - 本 API 直接返回 SwapChain 指针，简化调用链
     *
     * @param nativeHandle  原生窗口句柄（HWND / X11 Window / NSView）
     * @param handleType    句柄类型（NNNativeHandleType）
     * @param width         初始宽度
     * @param height        初始高度
     * @return SwapChain 指针（nullptr = 创建失败），同时内部注册 Surface
     *
     * @note 返回的 SwapChain 由内部 Surface 持有，调用者不应释放。
     *       销毁时需调用 ViewportSurfaceAPI.DestroySurface(surfaceId)。
     *       本 API 返回 0 作为 surfaceId（因为 void* 无法携带 ID），
     *       需要额外机制获取 surfaceId——建议仅用于临时/测试场景。
     */
    void* (NN_ENGINE_ABI_STDCALL *CreateViewportSurfaceWithSwapChain)(
        void* nativeHandle,
        uint32_t handleType,
        uint32_t width,
        uint32_t height);

} NNDiligentAPI;

#ifdef __cplusplus
} /* extern "C" */
#endif
```

### 2. 修改 `EngineAPIRegistry.h`

**路径**: `Engine/Source/Runtime/NNNativeEngineAPI/Include/EngineAPIRegistry.h`

修改点：
- 添加 `#include "DiligentAPI.h"`
- 在 `NNNativeEngineAPI` 结构体末尾追加 `NNDiligentAPI diligent;`
- 递增 `NN_NATIVE_ENGINE_API_LAYOUT_VERSION` 至 28

### 3. 新建 `DiligentRuntimeApi.cpp`

**路径**: `Engine/Source/Runtime/NNRuntimeEngineServices/Private/Diligent/DiligentRuntimeApi.cpp`

实现 4 个 API：
- `rt_diligent_getPrimaryDevice` - 通过 WindowRegistry → VGWindow → NNDiligentDevice 获取
- `rt_diligent_getPrimaryContext` - 同上路径
- `rt_diligent_getPrimarySwapChain` - 同上路径
- `rt_diligent_createViewportSurfaceWithSwapChain` - 复用 ViewportSurface 创建逻辑，返回 SwapChain

### 4. 修改 `RuntimeApiBuilders.h`

**路径**: `Engine/Source/Runtime/NNRuntimeEngineServices/Private/Internal/RuntimeApiBuilders.h`

添加：
```cpp
#include "NNNativeEngineAPI/Include/DiligentAPI.h"
void NNBuildDiligentRuntimeApi(NNDiligentAPI* api);
```

### 5. 修改 `NativeEngineRuntimeServices.cpp`（或等效注册文件）

在 Runtime API 注册流程中调用 `NNBuildDiligentRuntimeApi(&api->diligent);`

### 6. 修改 `NNNativeEngineApiTypes.cs`

**路径**: `Engine/Source/Managed/Runtime/Neverness.Runtime.Engine/NNNativeEngineApiTypes.cs`

添加 C# 结构体：
```csharp
/// <summary>
/// 與 Native <c>NNDiligentAPI</c> 對齊：Diligent 底层设备指针暴露。
/// v28 新增。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNDiligentApi
{
    /// <summary>获取主窗口的 Diligent IRenderDevice*。</summary>
    public delegate* unmanaged<void*> GetPrimaryDevice;

    /// <summary>获取主窗口的 Diligent IDeviceContext*。</summary>
    public delegate* unmanaged<void*> GetPrimaryContext;

    /// <summary>获取主窗口的 Diligent ISwapChain*。</summary>
    public delegate* unmanaged<void*> GetPrimarySwapChain;

    /// <summary>创建 ViewportSurface 并返回其 ISwapChain*。</summary>
    public delegate* unmanaged<void*, uint, uint, uint, void*> CreateViewportSurfaceWithSwapChain;
}
```

在 `NNNativeEngineApi` 结构体末尾追加：
```csharp
/// <summary>對應 C 聚合體成員 <c>diligent</c>（型別 <c>NNDiligentAPI</c>）；Diligent 底层指针（v28）。</summary>
public NNDiligentApi Diligent;
```

### 7. 修改 `NNNativeEngineApiConstants.cs`

**路径**: `Engine/Source/Managed/Runtime/Neverness.Runtime.Engine/NNNativeEngineApiConstants.cs`

递增 `LayoutVersion` 至 28。

## ABI 兼容性

- **追加模式**: 新子表追加在 `NNNativeEngineAPI` 结构体末尾，不破坏现有字段布局
- **layoutVersion**: 27 → 28，C# 端可据此判断是否可用 Diligent API
- **void* 返回**: Diligent 指针以 `void*` 形式跨 ABI 边界，C# 端需自行 cast

## 风险与注意事项

1. **Diligent 头文件依赖**: `DiligentAPI.h` 不包含 Diligent 头文件，使用 `void*` 隔离
2. **线程安全**: Diligent 要求单线程访问，API 调用者需确保在渲染线程执行
3. **生命周期**: 返回的指针生命周期与主窗口一致，主窗口销毁后不可使用
4. **CreateViewportSurfaceWithSwapChain**: 创建的 Surface 未注册到 ViewportSurfaceAPI 注册表，SwapChain 生命周期由 C# 端管理

## 实施顺序

1. 新建 `DiligentAPI.h`
2. 修改 `EngineAPIRegistry.h`（追加子表 + 递增版本号）
3. 新建 `DiligentRuntimeApi.cpp`（实现 4 个 API）
4. 修改 `RuntimeApiBuilders.h`（添加 builder 声明）
5. 修改注册流程（调用 builder）
6. 修改 C# 端 `NNNativeEngineApiTypes.cs` + `NNNativeEngineApiConstants.cs`
7. 编译验证

## 设计决策

- `CreateViewportSurfaceWithSwapChain` 不返回 surfaceId，SwapChain 生命周期由 C# 端管理
- 不需要 Shutdown 函数，资源释放由 C# 端负责
