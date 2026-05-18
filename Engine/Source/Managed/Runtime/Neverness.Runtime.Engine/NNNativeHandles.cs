namespace Neverness.Managed.Engine;

/// <summary>
/// 與 Native <c>EngineHandles.h</c> 對齊之 **不透明資源代碼**；禁止以託管物件包裝 Native 指標穿越 ABI。
/// </summary>
public readonly record struct NNTextureHandle(ulong Value);

public readonly record struct NNRenderTargetHandle(ulong Value);

public readonly record struct NNElementHandle(ulong Value);

public readonly record struct NNAudioHandle(ulong Value);

public readonly record struct NNAssetHandle(ulong Value);

public readonly record struct NNAsyncWaitHandle(ulong Value);

public readonly record struct NNEntityHandle(ulong Value);

/// <summary>
/// 與 Native <c>NNObjectHandle</c> 對齊之不透明控制代碼；託管 <see cref="Neverness.Managed.Object.VGObject"/> 與 Native 子系統之橋接鍵。
/// </summary>
public readonly record struct NNObjectHandle(ulong Value);
