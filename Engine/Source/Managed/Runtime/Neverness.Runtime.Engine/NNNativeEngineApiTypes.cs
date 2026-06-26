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
/// 與 Native <c>NNViewportRenderAPI</c> 逐欄位對齊（<c>ViewportRenderAPI.h</c>）。
/// v21 新增子表。已移除 RenderSceneToTexture / GetLastRenderedTexture / GetRenderStats（Legacy Scene）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNViewportRenderApi
{
	// ── RmlUI 渲染管理 ──
	public delegate* unmanaged<uint, uint, void> SetRmlUIViewportSize;
	public delegate* unmanaged<uint, int, int, int, int, uint, uint, uint, void> ProcessRmlUIInput;
	public delegate* unmanaged<ulong> GetLastRmluiTexture;

	// ── RmlUI 文档热重载 ──
	/// <summary>通知 native 端重新加载指定文档（UTF-8 VFS 路径，NUL 终结）。</summary>
	public delegate* unmanaged<byte*, void> ReloadRmlDocument;
	/// <summary>通知 native 端重新加载所有文档。</summary>
	public delegate* unmanaged<void> ReloadAllRmlDocuments;
}

/// <summary>
/// 视口 Surface API——管理 SwapChain 生命周期（Renderer 基础设施）。
///
/// 职责：Create / Destroy / Resize / Present
/// 与 ViewportRenderApi 分离：Surface 是 Renderer 基础设施，RenderViewport 是 Editor 功能。
/// 所有 Viewport（Scene / Material / Mesh / Texture / Shader）共用此 API。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNViewportSurfaceApi
{
	/// <summary>创建视口表面（SwapChain）。返回 surfaceId。</summary>
	public delegate* unmanaged<void*, NNNativeHandleType, uint, uint, ulong> CreateSurface;

	/// <summary>销毁视口表面。</summary>
	public delegate* unmanaged<ulong, void> DestroySurface;

	/// <summary>标记 Resize（Deferred，不立即执行 ResizeBuffers）。</summary>
	public delegate* unmanaged<ulong, uint, uint, void> MarkResize;

	/// <summary>帧末统一执行所有 Deferred Resize（Flush Resize Queue）。</summary>
	public delegate* unmanaged<void> FlushResizes;

	/// <summary>Present SwapChain（提交渲染结果到屏幕）。</summary>
	public delegate* unmanaged<ulong, void> Present;

	/// <summary>查询表面是否丢失（HWND 重建后需要 Recreate SwapChain）。</summary>
	public delegate* unmanaged<ulong, byte> IsSurfaceLost;

	/// <summary>重建丢失的表面（新 HWND）。成功返回 1。</summary>
	public delegate* unmanaged<ulong, void*, NNNativeHandleType, byte> RecreateSurface;

	/// <summary>通过渲染命令缓冲区渲染视口（C# Scene → Commands → C++ Renderer）。v29 新增。</summary>
	public delegate* unmanaged<ulong, void*, uint, byte> RenderViewportCommands;
}

/// <summary>
/// 原生窗口句柄类型——跨平台标识 HWND / X11 / NSView。
/// 与 C++ 端 NNNativeHandleType 枚举一一对应，不要用 C# 常量类镜像。
/// </summary>
public enum NNNativeHandleType : uint
{
	/// <summary>Win32 HWND（Windows）。</summary>
	Win32HWND = 0,

	/// <summary>X11 Window ID（Linux X11）。</summary>
	X11Window = 1,

	/// <summary>Wayland wl_surface（Linux Wayland）。</summary>
	WaylandSurface = 2,

	/// <summary>NSView 指针（macOS）。</summary>
	NSView = 3,
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
/// 與 Native <c>NNVfsFileSystemType</c> 對齊（<c>VfsAPI.h</c>）：VFS 文件系统类型。
/// </summary>
public enum NNVfsFileSystemType : uint
{
	/// <summary>NativeFileSystem（磁盘目录）。</summary>
	Native = 0,
	/// <summary>ZipFileSystem（.zip/.pak）。</summary>
	Zip = 1,
	/// <summary>MemoryFileSystem（内存）。</summary>
	Memory = 2,
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

	// ── handle 机制的文件系统管理 API（追加，不破坏旧 ABI） ──

	/// <summary>创建并挂载文件系统，返回 handle（0 失败）。</summary>
	public delegate* unmanaged<byte*, NNVfsFileSystemType, byte*, ulong> AddFileSystem;
	/// <summary>根据 handle 精确移除文件系统，非 0 成功。</summary>
	public delegate* unmanaged<ulong, int> RemoveFileSystem;
	/// <summary>查询 handle 是否仍在 VFS 中，非 0 存在。</summary>
	public delegate* unmanaged<ulong, int> HasFileSystem;
	/// <summary>移除 alias 下全部文件系统。</summary>
	public delegate* unmanaged<byte*, void> UnregisterAlias;
	/// <summary>查询 alias 是否已注册，非 0 已注册。</summary>
	public delegate* unmanaged<byte*, int> IsAliasRegistered;
}

/// <summary>
/// 與 Native <c>NNEventType</c> 對齊（<c>EventTypes.h</c>）：事件粗分类。
/// type = 粗分类（Engine-Level Category），subtype = 细分类（Per-Type Specific）。
/// </summary>
public enum NNEventType : uint
{
    None   = 0,
    Window = 1,  /* 窗口生命周期 */
    Input  = 2,  /* 鼠标/键盘/拖放 */
    System = 3,  /* 应用退出 */
    /* 4-63 预留给未来：SCENE=4, ENTITY=5, ASSET=6... */
}

/// <summary>
/// 與 Native <c>NNWindowEventSubtype</c> 對齊：窗口事件细分类。
/// </summary>
public enum NNWindowEventSubtype : uint
{
    None              =  0,
    Shown             =  1,
    Hidden            =  2,
    Exposed           =  3,
    Moved             =  4,
    Resized           =  5,
    SizeChanged       =  6,
    PixelSizeChanged  =  7,
    Minimized         =  8,
    Maximized         =  9,
    Restored          = 10,
    MouseEnter        = 11,
    MouseLeave        = 12,
    FocusGained       = 13,
    FocusLost         = 14,
    Close             = 15,
    DpiChanged        = 16,
    DisplayChanged    = 17,
    EnterFullscreen   = 18,
    LeaveFullscreen   = 19,
    Terminating       = 20,
    LowMemory         = 21,
}

/// <summary>
/// 與 Native <c>NNInputEventSubtype</c> 對齊：输入事件细分类（鼠标/键盘/拖放）。
/// </summary>
public enum NNInputEventSubtype : uint
{
    None            =  0,
    MouseMotion     =  1,
    MouseButtonDown =  2,
    MouseButtonUp   =  3,
    MouseWheel      =  4,
    KeyDown         =  5,
    KeyUp           =  6,
    TextInput       =  7,
    TextEditing     =  8,
    DropBegin       =  9,
    DropFile        = 10,
    DropText        = 11,
    DropComplete    = 12,
}

/// <summary>
/// 與 Native <c>NNSystemEventSubtype</c> 對齊：系统事件细分类。
/// </summary>
public enum NNSystemEventSubtype : uint
{
    None = 0,
    Quit = 1,
}

/// <summary>
/// 與 Native <c>NNEvent</c> 對齊（<c>EventTypes.h</c>）：128 字节 ABI 事件结构体。
/// Blittable，NativeAOT 兼容，零 GC Alloc。
/// </summary>
[StructLayout(LayoutKind.Sequential, Size = 128)]
public struct NNEvent
{
    public uint Type;           // NNEventType         4B  offset 0x00
    public uint Subtype;        // subtype enum        4B  offset 0x04
    public ulong Timestamp;     // SDL ticks ms        8B  offset 0x08
    public ulong Source;        // NNWindowHandle      8B  offset 0x10

    public int Data1;           // 通用数据 1          4B  offset 0x18
    public int Data2;           // 通用数据 2          4B  offset 0x1C
    public int Data3;           // 通用数据 3          4B  offset 0x20
    public int Data4;           // 通用数据 4          4B  offset 0x24

    public uint Flags;          // 事件标志            4B  offset 0x28
    public uint StringPoolIdx;  // String Pool 偏移    4B  offset 0x2C

    // reservedPad[20] = 80B padding (offset 0x30 - 0x7F)
    // Total = 128B
}

/// <summary>
/// 與 Native <c>NNEventAPI</c> 對齊（<c>EventAPI.h</c>）：事件队列函数表。
/// </summary>
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public unsafe struct NNEventApi
{
    public uint Size;

    public delegate* unmanaged<NNEvent*, uint> PollEvent;
    public delegate* unmanaged<NNEvent*, uint> PeekEvent;
    public delegate* unmanaged<NNEvent*, uint, uint> WaitEvent;
    public delegate* unmanaged<NNEvent*, byte**, ushort*, uint> GetEventString;
    public delegate* unmanaged<uint> GetQueueCount;
    public delegate* unmanaged<void> FlushEvents;
    public delegate* unmanaged<NNEvent*, uint> PushUserEvent;
}

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

	/// <summary>为外部创建的 SDL_Window 创建 Diligent 设备。返回 1=成功，0=失败。</summary>
	public delegate* unmanaged<void*, uint, uint, uint> CreateDeviceForWindow;

	/// <summary>从平台原生窗口句柄创建 Diligent 设备（绕过 SDL）。nativeHandle + handleType。</summary>
	public delegate* unmanaged<void*, uint, uint, uint, uint> CreateDeviceForNativeHandle;

	/// <summary>Present 主 SwapChain。</summary>
	public delegate* unmanaged<void> PresentPrimarySwapChain;

	/// <summary>获取主窗口的 INNRenderDevice*（NNRuntimeRender 接口）。</summary>
	public delegate* unmanaged<void*> GetPrimaryRenderDevice;
}

/// <summary>
/// 與 Native <c>NNNativeEngineAPI</c> 聚合體對齊（<c>EngineAPIRegistry.h</c>）；欄位順序須與 C 結構逐字節一致。
/// </summary>
/// <remarks>
/// **layout v39**：移除 ImGuiBackend 子表（独立为 NNRuntimeImGui 模块）。
/// </remarks>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct NNNativeEngineApi
{
	public uint LayoutVersion;
	public uint Reserved0;
	public NNRenderApi Render;
	public NNAudioApi Audio;
	public NNAsyncWaitApi AsyncWait;
	// Application/Window 已移除：C# Neverness.Runtime.Application 接管 (2026-06-24)
	/// <summary>對應 C 聚合體末尾成員 <c>vfs</c>（型別 <c>NNVfsAPI</c>）。</summary>
	public NNVfsApi Vfs;
	/// <summary>對應 C 聚合體成員 <c>events</c>（型別 <c>NNEventAPI</c>）；事件队列（Pull-Based）。</summary>
	public NNEventApi Events;
	// Input/RenderAsset 已移除：迁移至 C# (2026-06-24)
	/// <summary>對應 C 聚合體成員 <c>viewportRender</c>（型別 <c>NNViewportRenderAPI</c>）；场景渲染到离屏 FBO（v21）。</summary>
	public NNViewportRenderApi ViewportRender;
	/// <summary>對應 C 聚合體成員 <c>viewportSurface</c>（型別 <c>NNViewportSurfaceAPI</c>）；SwapChain 生命周期管理（v23）。</summary>
	public NNViewportSurfaceApi ViewportSurface;
	/// <summary>對應 C 聚合體成員 <c>diligent</c>（型別 <c>NNDiligentAPI</c>）；Diligent 底层指针（v28）。</summary>
	public NNDiligentApi Diligent;
	// ImGuiBackend 已移除：独立为 NNRuntimeImGui 模块 (2026-06-24)
}
