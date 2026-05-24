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

/// <summary>投影類型枚舉（與 Native <c>NNProjectionType</c> 對齊）。</summary>
public enum NNProjectionType : uint
{
	Perspective = 0,
	Orthographic = 1,
}

/// <summary>
/// 4x4 浮點矩陣，與 <c>glm::mat4</c> 內存佈局一致（列主序，64 字節）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct NNMat4
{
	public float M00, M10, M20, M30;
	public float M01, M11, M21, M31;
	public float M02, M12, M22, M32;
	public float M03, M13, M23, M33;
}

/// <summary>
/// 相機組件——blittable 結構體，與 Native <c>NNCameraComponent</c> 內存佈局一致。
/// ProjectionMatrix 由 Native NNCameraSystem 每幀計算，C# 端只讀。
/// TypeId = FNV-1a("Camera")，須與 Native BuiltinComponentRegistration.cpp 一致。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
[ComponentId(0x54D1B2A64667E32E, Name = "Camera")]
public struct NNCameraComponentData
{
	public NNProjectionType Projection;
	public float NearPlane;
	public float FarPlane;
	public uint _padding0;

	public float FovY;
	public float AspectRatio;
	public float OrthoWidth;
	public float OrthoHeight;

	public NNMat4 ProjectionMatrix;
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

// ── Editor Scene Snapshot 类型（与 EditorSceneAPI.h 对齐）──────────────────

/// <summary>
/// 快照头部——位于 buffer 开头，描述后续 Node 数组和名字池的布局。32 字节。
/// 与 Native <c>NNSceneSnapshotHeader</c>（<c>EditorSceneAPI.h</c>）逐字段对齐。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct NNSceneSnapshotHeader
{
	public uint  Magic;            // 0x56475343
	public uint  LayoutVersion;    // = 1
	public ulong HierarchyVersion;
	public uint  NodeCount;
	public uint  NamePoolBytes;
	public uint  RootCount;
	public uint  Pad;
}

/// <summary>
/// 场景层级节点快照——单个 Entity 的 Hierarchy 信息。40 字节。
/// 名字通过 NameOffset + NameLen 引用 Header 之后的 namePool。
/// 与 Native <c>NNSceneNodeSnapshot</c>（<c>EditorSceneAPI.h</c>）逐字段对齐。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct NNSceneNodeSnapshot
{
	public ulong Entity;
	public ulong Parent;
	public uint  Depth;
	public uint  ChildCount;
	public uint  NameOffset;
	public uint  NameLen;
	public uint  Flags;
	public uint  Pad;
}

/// <summary>
/// 脏节点条目——增量快照使用。16 字节。
/// 与 Native <c>NNDirtyNodeEntry</c>（<c>EditorSceneAPI.h</c>）逐字段对齐。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct NNDirtyNodeEntry
{
	public ulong Entity;
	public uint  ChangeFlags;
	public uint  Pad;
}

/// <summary>
/// Transform 快照数据——批量读取 Transform 组件。48 字节。
/// 与 Native <c>NNEditorTransformData</c>（<c>EditorSceneAPI.h</c>）逐字段对齐。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct NNEditorTransformData
{
	public ulong Entity;
	public float PosX, PosY, PosZ;
	public float RotX, RotY, RotZ, RotW;
	public float SclX, SclY, SclZ;
}

/// <summary>ChangeFlags 常量（与 Native <c>NN_DIRTY_*</c> 宏对齐）。</summary>
public static class SnapshotChangeFlags
{
	public const uint NameChanged     = 1u << 0;
	public const uint ParentChanged   = 1u << 1;
	public const uint ChildrenChanged = 1u << 2;
	public const uint ActiveChanged   = 1u << 3;
	public const uint FlagsChanged    = 1u << 4;
}

/// <summary>
/// Editor 专用场景查询函数表——独立于 <see cref="NNSceneApi"/>，layoutVersion = 2。
/// 与 Native <c>NNEditorSceneAPI</c>（<c>EditorSceneAPI.h</c>）逐字段对齐。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNEditorSceneApi
{
	public uint LayoutVersion;  // = 2

	public delegate* unmanaged<ulong, ulong> GetHierarchyVersion;
	public delegate* unmanaged<ulong, uint> GetSnapshotSize;
	public delegate* unmanaged<ulong, void*, uint, uint> GetHierarchySnapshot;

	public delegate* unmanaged<ulong, ulong> GetTransformVersion;
	public delegate* unmanaged<ulong, ulong*, uint, NNEditorTransformData*, uint> GetTransformSnapshot;

	// ── 增量快照（layoutVersion = 2 追加）──
	public delegate* unmanaged<ulong, void*, uint, uint> GetIncrementalSnapshot;
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

	// Phase 1 依赖管理（append-only，与 C++ NNAssetRegistryAPI 对齐）
	public delegate* unmanaged<NNGuid, NNGuid*, uint, int> SetDependencies;
	public delegate* unmanaged<NNGuid, NNGuid, int> AddDependency;
	public delegate* unmanaged<NNGuid, NNGuid, int> RemoveDependency;
	public delegate* unmanaged<NNGuid, uint> GetReverseDependencyCount;
	public delegate* unmanaged<NNGuid, uint, NNGuid*, int> GetReverseDependencyAt;
	public delegate* unmanaged<int> HasCycle;
	public delegate* unmanaged<uint> GetAssetCount;
	public delegate* unmanaged<uint> GetEdgeCount;
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
	public delegate* unmanaged<int> Initialize;
	public delegate* unmanaged<bool> PumpEvents;
	public delegate* unmanaged<void> Shutdown;
	public delegate* unmanaged<void> BeginFrame;
	public delegate* unmanaged<void> EndFrame;
}

/// <summary>
/// 與 Native <c>NNAssetManagerAPI</c> 對齊（<c>AssetManagerAPI.h</c>）：Runtime 資產管理器（同步/異步載入、卸載、包管理）。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public unsafe struct NNAssetManagerApi
{
	public delegate* unmanaged[Stdcall]<NNGuid, ulong, NNAssetHandle> LoadAssetSync;
	public delegate* unmanaged[Stdcall]<NNGuid, ulong, int, delegate* unmanaged[Stdcall]<NNAssetHandle, int, void*, void>, void*, NNAsyncWaitHandle> LoadAssetAsync;
	public delegate* unmanaged[Stdcall]<NNAssetHandle, void> UnloadAsset;
	public delegate* unmanaged[Stdcall]<NNGuid, void> UnloadAssetByGuid;
	public delegate* unmanaged[Stdcall]<NNAssetHandle, int> IsAssetLoaded;
	public delegate* unmanaged[Stdcall]<NNAssetHandle, int> IsAssetLoading;
	public delegate* unmanaged[Stdcall]<NNGuid, NNAssetHandle> GetAssetByGuid;
	public delegate* unmanaged[Stdcall]<NNAssetHandle, NNGuid> GetGuidByAsset;
	public delegate* unmanaged[Stdcall]<NNAssetHandle, void> AddRef;
	public delegate* unmanaged[Stdcall]<NNAssetHandle, void> ReleaseRef;
	public delegate* unmanaged[Stdcall]<NNAssetHandle, uint> GetRefCount;
	public delegate* unmanaged[Stdcall]<NNAssetHandle, void*> GetAssetData;
	public delegate* unmanaged[Stdcall]<NNAssetHandle, ulong> GetAssetDataSize;
	public delegate* unmanaged[Stdcall]<NNAssetHandle, uint> GetBlobCount;
	public delegate* unmanaged[Stdcall]<NNAssetHandle, uint, void*> GetBlobData;
	public delegate* unmanaged[Stdcall]<NNAssetHandle, uint, ulong> GetBlobSize;
	public delegate* unmanaged[Stdcall]<byte*, int> MountPackage;
	public delegate* unmanaged[Stdcall]<byte*, void> UnmountPackage;
	public delegate* unmanaged[Stdcall]<NNGuid, int> IsAssetInPackage;
	public delegate* unmanaged[Stdcall]<NNGuid, void> MarkForReload;
	public delegate* unmanaged[Stdcall]<void> ReloadMarkedAssets;
	public delegate* unmanaged[Stdcall]<ulong> GetLoadedAssetCount;
	public delegate* unmanaged[Stdcall]<ulong> GetTotalMemoryUsage;
}

/// <summary>
/// 與 Native <c>NNCookResultData</c> 對齊（<c>AssetCookerAPI.h</c>）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct NNCookResultData
{
	public int Success;
	public uint TotalAssets;
	public uint CookedAssets;
	public uint FailedAssets;
	public uint GeneratedPacks;
	public double ElapsedSeconds;
}

/// <summary>
/// 與 Native <c>NNAssetCookerAPI</c> 對齊（<c>AssetCookerAPI.h</c>）：資產編譯/打包器。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public unsafe struct NNAssetCookerApi
{
	public delegate* unmanaged[Stdcall]<ulong> CreateManifest;
	public delegate* unmanaged[Stdcall]<ulong, void> DestroyManifest;
	public delegate* unmanaged[Stdcall]<ulong, byte*, void> SetOutputRoot;
	public delegate* unmanaged[Stdcall]<ulong, byte*, void> SetLibraryRoot;
	public delegate* unmanaged[Stdcall]<ulong, NNGuid, ulong, byte*, uint, void> AddAsset;
	public delegate* unmanaged[Stdcall]<ulong, byte*, byte*, uint, byte*, void> AddGroup;
	public delegate* unmanaged[Stdcall]<ulong, NNCookResultData> Cook;
}

/// <summary>
/// 與 Native <c>NNNativeEngineAPI</c> 聚合體對齊（<c>EngineAPIRegistry.h</c>）；欄位順序須與 C 結構逐字節一致。
/// </summary>
/// <remarks>
/// **layout v17**：新增 <c>NNAssetManagerAPI</c>、<c>NNAssetCookerAPI</c> 子表。
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
	/// <summary>對應 C 聚合體成員 <c>editorScene</c>（型別 <c>NNEditorSceneAPI</c>）；Editor 快照查詢。</summary>
	public NNEditorSceneApi EditorScene;
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
	/// <summary>對應 C 聚合體成員 <c>assetManager</c>（型別 <c>NNAssetManagerAPI</c>）；Runtime 資產管理器。</summary>
	public NNAssetManagerApi AssetManager;
	/// <summary>對應 C 聚合體成員 <c>assetCooker</c>（型別 <c>NNAssetCookerAPI</c>）；資產編譯/打包器。</summary>
	public NNAssetCookerApi AssetCooker;
}
