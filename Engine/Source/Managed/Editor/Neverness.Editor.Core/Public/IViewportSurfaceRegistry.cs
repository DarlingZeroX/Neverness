namespace Neverness.Editor.Core;

/// <summary>
/// 视口表面注册表——管理多个 ViewportSurface 的生命周期。
///
/// 设计原则：
/// - 放在 Editor.Core，ImGuiFrontend 和 AvaloniaFrontend 共享
/// - 每个 Surface 持有独立 SwapChain
/// - Device/Context 全局共享，由 Renderer 管理
/// - Deferred Resize：MarkResize → FlushResizes（帧末统一执行）
/// - Surface Lost：HandleDestroyed → MarkLost → HandleCreated → Recreate
///
/// 使用场景：
/// - Scene Viewport（主视口）
/// - Material Preview（材质预览）
/// - Mesh Preview（模型预览）
/// - Texture Preview（纹理预览）
/// - Shader Preview（着色器预览）
/// </summary>
public interface IViewportSurfaceRegistry
{
    /// <summary>注册一个新的视口表面。</summary>
    /// <param name="nativeHandle">原生窗口句柄（HWND/X11/NSView）。</param>
    /// <param name="handleType">句柄类型（使用 NNNativeHandleType 枚举值）。</param>
    /// <param name="width">初始宽度。</param>
    /// <param name="height">初始高度。</param>
    /// <returns>表面 ID，用于后续操作。</returns>
    ulong Register(IntPtr nativeHandle, uint handleType, uint width, uint height);

    /// <summary>注销一个视口表面。</summary>
    void Unregister(ulong surfaceId);

    /// <summary>标记 Deferred Resize（不立即执行 ResizeBuffers，帧末统一 Flush）。</summary>
    void MarkResize(ulong surfaceId, uint width, uint height);

    /// <summary>帧末统一执行所有 Deferred Resize。</summary>
    void FlushResizes();

    /// <summary>渲染一个视口。</summary>
    /// <param name="surfaceId">表面 ID。</param>
    /// <param name="cameraId">相机 ID。
    /// TODO: cameraId 是临时设计。未来应改为 ViewportContext，
    /// 因为 Material Preview / Texture Preview / Shader Preview 可能没有 Camera。</param>
    void RenderViewport(ulong surfaceId, ulong cameraId);

    /// <summary>Present SwapChain（提交渲染结果到屏幕）。</summary>
    void Present(ulong surfaceId);

    /// <summary>
    /// 渲染视口到 SwapChain（完整路径：SceneRenderer → FBO → CopyTexture → SwapChain → Present）。
    /// </summary>
    /// <param name="surfaceId">表面 ID。</param>
    /// <param name="sceneHandle">场景句柄。</param>
    /// <param name="width">渲染宽度。</param>
    /// <param name="height">渲染高度。</param>
    /// <returns>是否成功。</returns>
    bool RenderViewport(ulong surfaceId, ulong sceneHandle, uint width, uint height);

    /// <summary>
    /// 标记表面丢失（HWND 被销毁/重建，如 Dock 浮动窗口）。
    /// 下次 RenderViewport 前需调用 RecreateSurface。
    /// </summary>
    void MarkSurfaceLost(ulong surfaceId);

    /// <summary>重建丢失的表面（新 HWND）。</summary>
    bool RecreateSurface(ulong surfaceId, IntPtr newNativeHandle, uint newHandleType);

    /// <summary>表面是否丢失。</summary>
    bool IsSurfaceLost(ulong surfaceId);

    /// <summary>表面是否已注册。</summary>
    bool IsRegistered(ulong surfaceId);

    /// <summary>获取已注册表面数量。</summary>
    int Count { get; }
}
