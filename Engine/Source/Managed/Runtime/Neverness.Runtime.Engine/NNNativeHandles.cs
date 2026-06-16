namespace Neverness.Runtime.Engine;

/// <summary>
/// 與 Native <c>EngineHandles.h</c> 對齊之 **不透明資源代碼**；禁止以託管物件包裝 Native 指標穿越 ABI。
/// </summary>
public readonly record struct NNTextureHandle(ulong Value);

public readonly record struct NNRenderTargetHandle(ulong Value);

public readonly record struct NNElementHandle(ulong Value);

public readonly record struct NNAudioHandle(ulong Value);

public readonly record struct NNAsyncWaitHandle(ulong Value);

public readonly record struct NNEntityHandle(ulong Value);

/// <summary>
/// 與 Native <c>NNObjectHandle</c> 對齊之不透明控制代碼；由 Native Object 子系統分配（託管僅持句柄，不复制对象模型）。
/// </summary>
public readonly record struct NNObjectHandle(ulong Value);
