using System.Runtime.InteropServices;

namespace Neverness.Runtime.Engine;

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
/// 與 Native <c>NNVec3</c> 對齊（<c>SceneAPI.h</c>）：三維向量（位置 / 縮放）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct NNVec3
{
	public float X;
	public float Y;
	public float Z;
}

/// <summary>
/// 與 Native <c>NNQuat</c> 對齊（<c>SceneAPI.h</c>）：四元數旋轉。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct NNQuat
{
	public float X;
	public float Y;
	public float Z;
	public float W;
}

/// <summary>
/// 與 Native <c>NNTransformData</c> 對齊（<c>SceneAPI.h</c>）：完整變換（位置 + 旋轉 + 縮放）。
/// TypeId = FNV-1a("Transform")，須與 Native BuiltinComponentRegistration.cpp 一致。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
[ComponentId(0xC1FFF4F356DFB2FB, Name = "Transform")]
public struct NNTransformData
{
	public NNVec3 Position;
	public NNQuat Rotation;
	public NNVec3 Scale;
}

/// <summary>
/// 與 Native <c>NNSceneResult</c> 對齊（<c>SceneAPI.h</c>）：場景操作結果碼。
/// </summary>
public enum NNSceneResult : int
{
	/// <summary>成功。</summary>
	Ok = 0,
	/// <summary>實體 / 場景未找到。</summary>
	NotFound = 1,
	/// <summary>句柄無效。</summary>
	Invalid = 2,
	/// <summary>輸出緩衝區容量不足。</summary>
	BufferSmall = 3,
	/// <summary>序列化 / 反序列化 I/O 錯誤。</summary>
	IO = 4,
}

/// <summary>
/// 與 Native <c>NNSceneAPI</c> 對齊（<c>SceneAPI.h</c>，layoutVersion = 6）。
/// 首字段為 <c>LayoutVersion</c>，後接 17 個函數指標。
/// NNSceneHandle = ulong（uint64），componentTypeId = ulong（FNV-1a name hash）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNSceneApi
{
	public uint LayoutVersion;  // = 6

	// 场景管理
	public delegate* unmanaged<ulong*, NNSceneResult> CreateScene;
	public delegate* unmanaged<ulong, NNSceneResult> DestroyScene;
	public delegate* unmanaged<ulong, float, NNSceneResult> TickSystems;

	// 实体 CRUD
	public delegate* unmanaged<ulong, ulong*, NNSceneResult> CreateEntity;
	public delegate* unmanaged<ulong, ulong, NNSceneResult> DestroyEntity;

	// 组件操作（FNV-1a name hash）
	public delegate* unmanaged<ulong, ulong, ulong, NNSceneResult> AddComponent;
	public delegate* unmanaged<ulong, ulong, ulong, NNSceneResult> RemoveComponent;
	public delegate* unmanaged<ulong, ulong, ulong, int*, NNSceneResult> HasComponent;
	public delegate* unmanaged<ulong, ulong, ulong, void*, uint, NNSceneResult> GetComponent;
	public delegate* unmanaged<ulong, ulong, ulong, void*, uint, NNSceneResult> SetComponent;

	// 层级
	public delegate* unmanaged<ulong, ulong, ulong, NNSceneResult> SetParent;
	public delegate* unmanaged<ulong, ulong, ulong*, NNSceneResult> GetParent;

	// 序列化（经 VFS 路径）
	public delegate* unmanaged<ulong, byte*, NNSceneResult> SerializeScene;
	public delegate* unmanaged<ulong*, byte*, NNSceneResult> DeserializeScene;

	// 批量查询（layoutVersion = 6 追加）
	public delegate* unmanaged<ulong, ulong, ulong*, uint, uint*, NNSceneResult> QueryEntities;
	public delegate* unmanaged<ulong, ulong, ulong*, uint, void*, uint, NNSceneResult> QueryComponents;
	public delegate* unmanaged<ulong, ulong, ulong, uint*, NNSceneResult> QueryCount2;
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
/// 與 <see cref="NNSceneApi"/> 所操作之場景句柄（Native <c>NNEntityHandle</c>）語意分離。
/// **GetRuntimeTick** 僅反映 **EntitySubsystem** 是否已被 **NNEngineRuntime::Tick** 驅動（與 **Stub** 表區分）。
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
/// 與 Native <c>NNWindowDesc</c> 對齊（<c>WindowAPI.h</c>）：創建窗口時的描述塊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNWindowDesc
{
	public byte* Title;
	public int Width;
	public int Height;
	public bool Resizable;
	public bool Maximized;
	public bool Hidden;
}

/// <summary>
/// 與 Native <c>NNWindowAPI</c> 對齊（<c>WindowAPI.h</c>）：多窗口、Native 句柄。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public unsafe struct NNWindowApi
{
	public uint Size;
	public delegate* unmanaged<NNWindowDesc*, ulong> Create;
	public delegate* unmanaged<ulong, void> Destroy;
	public delegate* unmanaged<ulong, byte*, void> SetTitle;
	public delegate* unmanaged<ulong, int, int, void> SetSize;
	public delegate* unmanaged<ulong, int*, int*, void> GetSize;
	public delegate* unmanaged<ulong, int, int, void> SetPosition;
	public delegate* unmanaged<ulong, int*, int*, void> GetPosition;
	public delegate* unmanaged<ulong, bool, void> SetResizable;
	public delegate* unmanaged<ulong, void> Maximize;
	public delegate* unmanaged<ulong, void> Minimize;
	public delegate* unmanaged<ulong, void> Restore;
	public delegate* unmanaged<ulong, void> Show;
	public delegate* unmanaged<ulong, void> Hide;
	public delegate* unmanaged<ulong, void*> GetNativeHandle;
}

/// <summary>
/// 與 Native <c>NNVfsAPI</c> 對齊（<c>VfsAPI.h</c>）：VFS 文本/二进制 IO；缓冲区由 Native <c>malloc</c>，须 <see cref="FreeBuffer"/>。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public unsafe struct NNVfsApi
{
	public uint Size;
	/// <summary>非 0 成功；<paramref name="outText"/> 为 malloc 缓冲区。</summary>
	public delegate* unmanaged<byte*, byte**, int> ReadText;
	public delegate* unmanaged<byte*, byte*, int> WriteText;
	public delegate* unmanaged<byte*, byte**, uint*, int> ReadBytes;
	public delegate* unmanaged<void*, void> FreeBuffer;
	public delegate* unmanaged<byte*, byte*, byte**, int> GetRelativePath;
	public delegate* unmanaged<byte*, int> RebuildNativeFileSystemFiles;
	public delegate* unmanaged<byte*, byte**, int> GetAbsolutePath;
	/// <summary>非 0 成功；将二进制缓冲区写入 VFS 路径。</summary>
	public delegate* unmanaged<byte*, byte*, ulong, int> WriteBufferToFile;
}

/// <summary>
/// 與 Native <c>NNApplicationAPI</c> 對齊（<c>ApplicationAPI.h</c>）：SDL 子系統、事件泵、幀邊界。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public unsafe struct NNApplicationApi
{
	public uint Size;
	public delegate* unmanaged<bool> Initialize;
	public delegate* unmanaged<bool> PumpEvents;
	public delegate* unmanaged<void> Shutdown;
	public delegate* unmanaged<void> BeginFrame;
	public delegate* unmanaged<void> EndFrame;
}

/// <summary>
/// 與 Native <c>NNNativeEngineAPI</c> 聚合體對齊（<c>EngineAPIRegistry.h</c>）；欄位順序須與 C 結構逐字節一致。
/// </summary>
/// <remarks>
/// **layout v10** 起 <see cref="Vfs"/> 含 <c>getAbsolutePath</c> 等；若 Native 遞增 layout 而託管未同步，<see cref="EngineNativeApiBootstrap.InstallFromNativeApiTable"/> 將拒絕快取。
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
	/// <summary>對應 C 聚合體成員 <c>entity</c>（型別 <c>NNEntityAPI</c>）；語義見 <see cref="NNEntityApi"/> 之 remarks。</summary>
	public NNEntityApi Entity;
	/// <summary>對應 C 聚合體成員 <c>application</c>（型別 <c>NNApplicationAPI</c>）。</summary>
	public NNApplicationApi Application;
	/// <summary>對應 C 聚合體成員 <c>window</c>（型別 <c>NNWindowAPI</c>）。</summary>
	public NNWindowApi Window;
	/// <summary>對應 C 聚合體末尾成員 <c>vfs</c>（型別 <c>NNVfsAPI</c>）。</summary>
	public NNVfsApi Vfs;
}
