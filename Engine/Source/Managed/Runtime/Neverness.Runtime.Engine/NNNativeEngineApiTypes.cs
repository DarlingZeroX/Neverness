using System.Runtime.InteropServices;

namespace Neverness.Managed.Engine;

/// <summary>
/// 與 Native <c>NNRenderAPI</c> 逐欄位對齊（<c>RenderAPI.h</c>）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNRenderApi
{
	public delegate* unmanaged<uint, uint, ulong> CreateTexture;
	public delegate* unmanaged<ulong, byte*, nuint, void> UploadTexture;
	public delegate* unmanaged<uint, uint, ulong> CreateRenderTarget;
}

/// <summary>
/// 與 Native <c>NNUIAPI</c> 對齊；語義層抽象 RmlUi，不暴露任何 Rml 原生指標。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNUIApi
{
	public delegate* unmanaged<ulong, byte*, void> SetDialogueText;
	public delegate* unmanaged<ulong, int, void> SetElementVisible;
}

/// <summary>
/// 與 Native <c>NNAudioAPI</c> 對齊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNAudioApi
{
	public delegate* unmanaged<byte*, ulong> PlayBgm;
	public delegate* unmanaged<byte*, ulong> PlayVoice;
}

/// <summary>
/// 與 Native <c>NNAssetAPI</c> 對齊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNAssetApi
{
	public delegate* unmanaged<byte*, ulong> LoadAsset;
	public delegate* unmanaged<ulong, void> UnloadAsset;
	public delegate* unmanaged<byte*, ulong> LoadTexture;
	public delegate* unmanaged<byte*, ulong> LoadAudio;
}

/// <summary>
/// 與 Native <c>NNInputAPI</c> 對齊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNInputApi
{
	public delegate* unmanaged<int, int> IsKeyPressed;
}

/// <summary>
/// 與 Native <c>NNTransform3</c> 對齊（<c>EngineTypes.h</c>）：position[3]、rotation[3]、scale[3]。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct NNTransform3
{
	public float PositionX;
	public float PositionY;
	public float PositionZ;
	public float RotationX;
	public float RotationY;
	public float RotationZ;
	public float ScaleX;
	public float ScaleY;
	public float ScaleZ;
}

/// <summary>
/// 與 Native <c>NNSceneAPI</c> 對齊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNSceneApi
{
	public delegate* unmanaged<byte*, int> LoadScene;
	public delegate* unmanaged<byte*, ulong> Spawn;
	public delegate* unmanaged<ulong, void> Destroy;
	public delegate* unmanaged<byte*, ulong> Find;
	public delegate* unmanaged<ulong, int, void> Activate;
	public delegate* unmanaged<byte*, int> UnloadScene;
	public delegate* unmanaged<byte*, nuint, int> GetActiveSceneName;
	public delegate* unmanaged<ulong, ulong, void> SetParent;
	public delegate* unmanaged<ulong, ulong> GetParent;
	public delegate* unmanaged<ulong, uint> GetChildCount;
	public delegate* unmanaged<ulong, uint, ulong> GetChildAt;
	public delegate* unmanaged<ulong, NNTransform3*, void> GetTransform;
	public delegate* unmanaged<ulong, NNTransform3*, void> SetTransform;
	public delegate* unmanaged<ulong, byte*, int> SetEntityName;
	public delegate* unmanaged<ulong, byte*, nuint, int> GetEntityName;
}

/// <summary>
/// 與 Native <c>NNTimingAPI</c> 對齊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNTimingApi
{
	public delegate* unmanaged<float> GetDeltaTime;
	public delegate* unmanaged<float> GetTotalTime;
	public delegate* unmanaged<ulong> GetFrameIndex;
}

/// <summary>
/// 與 Native <c>NNAsyncWaitAPI</c> 對齊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNAsyncWaitApi
{
	public delegate* unmanaged<ulong> CreateWait;
	public delegate* unmanaged<ulong, int> TryComplete;
	public delegate* unmanaged<ulong, void> ReleaseWait;
}

/// <summary>
/// 與 Native <c>NNGuid</c> 對齊之 128-bit 資產 GUID（<c>EngineTypes.h</c>）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct NNGuid
{
	public ulong High;
	public ulong Low;
}

/// <summary>
/// 與 Native <c>NNObjectAPI</c> 對齊（<c>ObjectAPI.h</c>）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNObjectApi
{
	public delegate* unmanaged<byte*, ulong> CreateObject;
	public delegate* unmanaged<ulong, void> DestroyObject;
	public delegate* unmanaged<ulong, void> RetainObject;
	public delegate* unmanaged<ulong, void> ReleaseObject;
	public delegate* unmanaged<ulong, uint> GetRefCount;
	public delegate* unmanaged<ulong, int> IsAlive;
	public delegate* unmanaged<ulong, byte*, nuint, int> GetTypeName;
}

/// <summary>
/// 與 Native <c>NNAssetRegistryAPI</c> 對齊（<c>AssetRegistryAPI.h</c>）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNAssetRegistryApi
{
	public delegate* unmanaged<byte*, NNGuid, int> RegisterAsset;
	public delegate* unmanaged<NNGuid, int> UnregisterByGuid;
	public delegate* unmanaged<byte*, int> UnregisterByPath;
	public delegate* unmanaged<NNGuid, byte*, nuint, int> ResolvePathByGuid;
	public delegate* unmanaged<byte*, NNGuid*, int> ResolveGuidByPath;
	public delegate* unmanaged<NNGuid, uint> GetDependencyCount;
	public delegate* unmanaged<NNGuid, uint, NNGuid*, int> GetDependencyAt;
	public delegate* unmanaged<byte*, NNGuid> ImportAsset;
}

/// <summary>
/// 與 Native <c>NNEntityAPI</c> 對齊（<c>EntityAPI.h</c>）。**layout v5** 起含 <c>getRuntimeTick</c>（Runtime 覆寫後隨 Tick 遞增；Stub 恒為 **0**）。
/// </summary>
/// <remarks>
/// 與 <see cref="NNSceneApi"/> 所操作之場景句柄（Native <c>NNEntityHandle</c>）語意分離；與託管 <c>Neverness.Managed.Entity.EntityHandle</c> 無數值映射。
/// **GetRuntimeTick** 僅反映 **EntitySubsystem** 是否已被 **NNEngineRuntime::Tick** 驅動（與 **Stub** 表區分）；**不**代表託管 **EntityWorld** 元件資料已與 Kernel 鏡像或自動同步（總覽 **§2.7.1** 資料策略）。
/// 欄位順序須與 C 端 <c>typedef struct NNEntityAPI</c>（<c>EntityAPI.h</c>）逐欄一致。
/// </remarks>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNEntityApi
{
	/// <summary>對應 Native <c>NNEntityGetServiceAbiTokenFn</c>；Stub 返回 <see cref="NNNativeEngineApiConstants.EntityServiceAbiToken"/>（ASCII「NNEn」小端）。</summary>
	public delegate* unmanaged<uint> GetServiceAbiToken;
	/// <summary>對應 Native <c>NNEntityGetRuntimeTickFn</c>；Runtime 轉發至 <c>EntitySubsystem</c> 之單調幀計數。</summary>
	public delegate* unmanaged<ulong> GetRuntimeTick;
}

/// <summary>
/// 與 Native <c>NNNativeEngineAPI</c> 聚合體對齊（<c>EngineAPIRegistry.h</c>）；欄位順序須與 C 結構逐字節一致。
/// </summary>
/// <remarks>
/// **layout v5** 起末尾為 <see cref="Entity"/>（<c>NNEntityAPI</c>，含 **getRuntimeTick**）；若 Native 遞增 <c>NN_NATIVE_ENGINE_API_LAYOUT_VERSION</c> 而託管未同步，<see cref="EngineNativeApiBootstrap.InstallFromNativeApiTable"/> 將拒絕快取。
/// </remarks>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNNativeEngineApi
{
	public uint LayoutVersion;
	public uint Reserved0;
	public NNRenderApi Render;
	public NNUIApi UI;
	public NNAudioApi Audio;
	public NNAssetApi Asset;
	public NNInputApi Input;
	public NNSceneApi Scene;
	public NNTimingApi Timing;
	public NNAsyncWaitApi AsyncWait;
	public NNObjectApi Object;
	public NNAssetRegistryApi AssetRegistry;
	/// <summary>對應 C 聚合體末尾成員 <c>entity</c>（型別 <c>NNEntityAPI</c>）；語義見 <see cref="NNEntityApi"/> 之 remarks。</summary>
	public NNEntityApi Entity;
}
