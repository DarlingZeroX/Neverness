namespace VisionGal.Managed.Engine;

/// <summary>
/// 與 Native <c>EngineHandles.h</c> 對齊之 **不透明資源代碼**；禁止以託管物件包裝 Native 指標穿越 ABI。
/// </summary>
public readonly record struct VGTextureHandle(ulong Value);

public readonly record struct VGRenderTargetHandle(ulong Value);

public readonly record struct VGElementHandle(ulong Value);

public readonly record struct VGAudioHandle(ulong Value);

public readonly record struct VGAssetHandle(ulong Value);

public readonly record struct VGAsyncWaitHandle(ulong Value);

public readonly record struct VGEntityHandle(ulong Value);

/// <summary>
/// 與 Native <c>VGObjectHandle</c> 對齊之不透明控制代碼；託管 <see cref="VisionGal.Managed.Object.VGObject"/> 與 Native 子系統之橋接鍵。
/// </summary>
public readonly record struct VGObjectHandle(ulong Value);
