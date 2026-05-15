using System.Runtime.InteropServices;

namespace VisionGal.Managed.Engine;

/// <summary>
/// 與 Native <c>VGRenderAPI</c> 逐欄位對齊（<c>RenderAPI.h</c>）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct VGRenderApi
{
	public delegate* unmanaged<uint, uint, ulong> CreateTexture;
	public delegate* unmanaged<ulong, byte*, nuint, void> UploadTexture;
	public delegate* unmanaged<uint, uint, ulong> CreateRenderTarget;
}

/// <summary>
/// 與 Native <c>VGUIAPI</c> 對齊；語義層抽象 RmlUi，不暴露任何 Rml 原生指標。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct VGUIApi
{
	public delegate* unmanaged<ulong, byte*, void> SetDialogueText;
	public delegate* unmanaged<ulong, int, void> SetElementVisible;
}

/// <summary>
/// 與 Native <c>VGAudioAPI</c> 對齊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct VGAudioApi
{
	public delegate* unmanaged<byte*, ulong> PlayBgm;
	public delegate* unmanaged<byte*, ulong> PlayVoice;
}

/// <summary>
/// 與 Native <c>VGAssetAPI</c> 對齊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct VGAssetApi
{
	public delegate* unmanaged<byte*, ulong> LoadAsset;
	public delegate* unmanaged<ulong, void> UnloadAsset;
	public delegate* unmanaged<byte*, ulong> LoadTexture;
	public delegate* unmanaged<byte*, ulong> LoadAudio;
}

/// <summary>
/// 與 Native <c>VGInputAPI</c> 對齊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct VGInputApi
{
	public delegate* unmanaged<int, int> IsKeyPressed;
}

/// <summary>
/// 與 Native <c>VGTransform3</c> 對齊（<c>EngineTypes.h</c>）：position[3]、rotation[3]、scale[3]。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct VGTransform3
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
/// 與 Native <c>VGSceneAPI</c> 對齊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct VGSceneApi
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
	public delegate* unmanaged<ulong, VGTransform3*, void> GetTransform;
	public delegate* unmanaged<ulong, VGTransform3*, void> SetTransform;
	public delegate* unmanaged<ulong, byte*, int> SetEntityName;
	public delegate* unmanaged<ulong, byte*, nuint, int> GetEntityName;
}

/// <summary>
/// 與 Native <c>VGTimingAPI</c> 對齊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct VGTimingApi
{
	public delegate* unmanaged<float> GetDeltaTime;
	public delegate* unmanaged<float> GetTotalTime;
	public delegate* unmanaged<ulong> GetFrameIndex;
}

/// <summary>
/// 與 Native <c>VGAsyncWaitAPI</c> 對齊。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct VGAsyncWaitApi
{
	public delegate* unmanaged<ulong> CreateWait;
	public delegate* unmanaged<ulong, int> TryComplete;
	public delegate* unmanaged<ulong, void> ReleaseWait;
}

/// <summary>
/// 與 Native <c>VGGuid</c> 對齊之 128-bit 資產 GUID（<c>EngineTypes.h</c>）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct VGGuid
{
	public ulong High;
	public ulong Low;
}

/// <summary>
/// 與 Native <c>VGObjectAPI</c> 對齊（<c>ObjectAPI.h</c>）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct VGObjectApi
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
/// 與 Native <c>VGAssetRegistryAPI</c> 對齊（<c>AssetRegistryAPI.h</c>）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct VGAssetRegistryApi
{
	public delegate* unmanaged<byte*, VGGuid, int> RegisterAsset;
	public delegate* unmanaged<VGGuid, int> UnregisterByGuid;
	public delegate* unmanaged<byte*, int> UnregisterByPath;
	public delegate* unmanaged<VGGuid, byte*, nuint, int> ResolvePathByGuid;
	public delegate* unmanaged<byte*, VGGuid*, int> ResolveGuidByPath;
	public delegate* unmanaged<VGGuid, uint> GetDependencyCount;
	public delegate* unmanaged<VGGuid, uint, VGGuid*, int> GetDependencyAt;
	public delegate* unmanaged<byte*, VGGuid> ImportAsset;
}

/// <summary>
/// 與 Native <c>VGNativeEngineAPI</c> 聚合體對齊（<c>EngineAPIRegistry.h</c>）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct VGNativeEngineApi
{
	public uint LayoutVersion;
	public uint Reserved0;
	public VGRenderApi Render;
	public VGUIApi UI;
	public VGAudioApi Audio;
	public VGAssetApi Asset;
	public VGInputApi Input;
	public VGSceneApi Scene;
	public VGTimingApi Timing;
	public VGAsyncWaitApi AsyncWait;
	public VGObjectApi Object;
	public VGAssetRegistryApi AssetRegistry;
}
