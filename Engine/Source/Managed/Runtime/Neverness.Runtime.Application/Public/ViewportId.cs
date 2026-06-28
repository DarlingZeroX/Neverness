// Neverness.Runtime.Application — 视口标识符。
// 统一管理 WindowHandle、surfaceId、IViewportService 的关联关系。
// 内化 Register/RecreateSurface/MarkResize 等渲染表面操作。
// 提供每帧输入事件处理（Update）。

using Neverness.Rendering.Core;
using Neverness.Runtime.Application.Private;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Application.Public;

/// <summary>
/// 视口标识符——统一管理 SDL 窗口、渲染表面、视口服务的关联关系。
///
/// 设计目的：
/// - 解决 WindowHandle、surfaceId、IViewportService 三者没有直接关联的问题
/// - 提供统一的标识符，方便事件路由和渲染调用
/// - 内化渲染表面操作（Register/RecreateSurface/MarkResize）
/// - 提供每帧输入事件处理（Update）
///
/// 使用场景：
/// - SDL 输入事件 → 通过 WindowHandle 找到 ViewportId → 调用 IViewportService
/// - 渲染调用 → 通过 surfaceId 找到 ViewportId → 调用 IViewportService
/// - 窗口管理 → 通过 WindowHandle 找到 ViewportId → 管理生命周期
/// - 表面注册 → ViewportId.Register() → 自动设置 SurfaceId
/// - 每帧更新 → ViewportId.Update() → 处理输入事件
/// </summary>
public class ViewportId : IEquatable<ViewportId>
{
    /// <summary>无效视口标识符。</summary>
    public static readonly ViewportId Invalid = new(WindowHandle.Invalid, 0, null);

    /// <summary>SDL 窗口句柄（SDL_WindowID）。</summary>
    public WindowHandle WindowHandle { get; }

    /// <summary>渲染表面 ID（Register() 后自动设置）。</summary>
    public ulong SurfaceId { get; private set; }

    /// <summary>视口服务（用于收集渲染命令和场景操作）。</summary>
    public IViewportService? ViewportService { get; private set; }

    /// <summary>是否已注册到渲染表面注册表。</summary>
    public bool IsRegistered => SurfaceId != 0;

    /// <summary>是否有效（SDL 窗口有效且已注册）。</summary>
    public bool IsValid => WindowHandle.IsValid;

    /// <summary>获取对应的 SdlWindow（可能为 null）。</summary>
    public SdlWindow? Window => SdlWindowManager.Resolve(WindowHandle);

    // ── 输入事件缓冲 ──
    private readonly List<SDL.SDL_Event> _inputEvents = new();

    /// <summary>
    /// 创建视口标识符。
    /// </summary>
    /// <param name="windowHandle">SDL 窗口句柄。</param>
    /// <param name="surfaceId">渲染表面 ID（默认为 0，表示未注册）。</param>
    /// <param name="viewportService">视口服务（可选，后续可设置）。</param>
    public ViewportId(WindowHandle windowHandle, ulong surfaceId = 0, IViewportService? viewportService = null)
    {
        WindowHandle = windowHandle;
        SurfaceId = surfaceId;
        ViewportService = viewportService;
    }

    /// <summary>
    /// 设置视口服务。
    ///
    /// 调用时机：Controller 初始化完成后。
    /// </summary>
    /// <param name="viewportService">视口服务。</param>
    public void SetViewportService(IViewportService viewportService)
    {
        ViewportService = viewportService;
    }

    /// <summary>
    /// 推送输入事件到缓冲区。
    ///
    /// 调用时机：SDL 事件泵接收到属于该窗口的输入事件时。
    /// 由 SdlEventBridge 或 ViewportIdManager 调用。
    /// </summary>
    /// <param name="e">SDL 事件。</param>
    public void PushInputEvent(SDL.SDL_Event e)
    {
        _inputEvents.Add(e);
    }

    /// <summary>
    /// 每帧更新——处理输入事件。
    ///
    /// 调用时机：每帧主循环开始时，由 ViewportAvaloniaView.OnMainThreadRender() 调用。
    ///
    /// 工作流程：
    /// 1. 从输入事件缓冲区取出所有事件
    /// 2. 分发到 IViewportService 处理
    /// 3. 清空缓冲区
    /// </summary>
    public void Update()
    {
        if (_inputEvents.Count == 0)
        {
            return;
        }

        // 处理所有缓冲的输入事件
        foreach (var e in _inputEvents)
        {
            DispatchInputEvent(e);
        }

        // 清空缓冲区
        _inputEvents.Clear();
    }

    /// <summary>
    /// 分发单个输入事件到 IViewportService。
    /// </summary>
    /// <param name="e">SDL 事件。</param>
    private void DispatchInputEvent(SDL.SDL_Event e)
    {
        if (ViewportService == null)
        {
            return;
        }

        switch (e.Type)
        {
            // 键盘事件
            case SDL.SDL_EventType.SDL_EVENT_KEY_DOWN:
                // TODO: 转发给 ViewportService
                // ViewportService.OnKeyDown(e.key.key, e.key.scancode, e.key.repeat);
                break;

            case SDL.SDL_EventType.SDL_EVENT_KEY_UP:
                // TODO: 转发给 ViewportService
                // ViewportService.OnKeyUp(e.key.key, e.key.scancode);
                break;

            // 鼠标事件
            case SDL.SDL_EventType.SDL_EVENT_MOUSE_MOTION:
                // TODO: 转发给 ViewportService
                // ViewportService.OnMouseMove(e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel);
                break;

            case SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_DOWN:
                // TODO: 转发给 ViewportService
                // ViewportService.OnMouseDown(e.button.button, e.button.x, e.button.y, e.button.clicks);
                break;

            case SDL.SDL_EventType.SDL_EVENT_MOUSE_BUTTON_UP:
                // TODO: 转发给 ViewportService
                // ViewportService.OnMouseUp(e.button.button, e.button.x, e.button.y, e.button.clicks);
                break;

            case SDL.SDL_EventType.SDL_EVENT_MOUSE_WHEEL:
                // TODO: 转发给 ViewportService
                // ViewportService.OnMouseWheel(e.wheel.x, e.wheel.y, e.wheel.direction);
                break;

            // 游戏手柄事件
            case SDL.SDL_EventType.SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                // TODO: 转发给 ViewportService
                // ViewportService.OnGamepadButtonDown(e.gbutton.which, e.gbutton.button);
                break;

            case SDL.SDL_EventType.SDL_EVENT_GAMEPAD_BUTTON_UP:
                // TODO: 转发给 ViewportService
                // ViewportService.OnGamepadButtonUp(e.gbutton.which, e.gbutton.button);
                break;

            case SDL.SDL_EventType.SDL_EVENT_GAMEPAD_AXIS_MOTION:
                // TODO: 转发给 ViewportService
                // ViewportService.OnGamepadAxis(e.gaxis.which, e.gaxis.axis, e.gaxis.value);
                break;
        }
    }

    /// <summary>
    /// 注册渲染表面。
    ///
    /// 工作流程：
    /// 1. 从 SdlWindowManager 获取 SDL 窗口
    /// 2. 获取原生句柄（HWND/NSView/X11）
    /// 3. 调用 IViewportSurfaceRegistry.Register()
    /// 4. 自动设置 SurfaceId
    ///
    /// 调用时机：
    /// - NativeControlHostSurface.OnHandleCreated 后
    /// - ViewportAvaloniaView 初始化时
    /// </summary>
    /// <param name="surfaceRegistry">渲染表面注册表。</param>
    /// <param name="width">初始宽度。</param>
    /// <param name="height">初始高度。</param>
    /// <returns>是否注册成功。</returns>
    public bool Register(IViewportSurfaceRegistry surfaceRegistry, uint width, uint height)
    {
        if (IsRegistered)
        {
            Console.WriteLine($"[ViewportId] 已注册，跳过重复注册: {this}");
            return true;
        }

        // 从 SdlWindowManager 获取 SDL 窗口
        var sdlWindow = SdlWindowManager.Resolve(WindowHandle);
        if (sdlWindow == null)
        {
            Console.Error.WriteLine($"[ViewportId] Register 失败: 无法找到 SDL 窗口 {WindowHandle}");
            return false;
        }

        // 获取原生句柄
        var nativeHandle = sdlWindow.NativeHandle;
        if (nativeHandle == IntPtr.Zero)
        {
            Console.Error.WriteLine($"[ViewportId] Register 失败: 原生句柄无效");
            return false;
        }

        // 确定句柄类型
        uint handleType = GetHandleType(sdlWindow);
        if (handleType == 0)
        {
            Console.Error.WriteLine($"[ViewportId] Register 失败: 不支持的句柄类型");
            return false;
        }

        // 注册到渲染表面注册表
        SurfaceId = surfaceRegistry.Register(nativeHandle, handleType, width, height);
        if (SurfaceId == 0)
        {
            Console.Error.WriteLine($"[ViewportId] Register 失败: surfaceRegistry.Register 返回 0");
            return false;
        }

        Console.WriteLine($"[ViewportId] 注册成功: {this}");
        return true;
    }

    /// <summary>
    /// 重建丢失的渲染表面。
    ///
    /// 调用时机：
    /// - Surface Lost 后，获取到新的原生句柄时
    /// - Dock 浮动窗口导致 HWND 重建时
    /// </summary>
    /// <param name="surfaceRegistry">渲染表面注册表。</param>
    /// <returns>是否重建成功。</returns>
    public bool RecreateSurface(IViewportSurfaceRegistry surfaceRegistry)
    {
        if (!IsRegistered)
        {
            Console.Error.WriteLine($"[ViewportId] RecreateSurface 失败: 未注册");
            return false;
        }

        // 从 SdlWindowManager 获取 SDL 窗口
        var sdlWindow = SdlWindowManager.Resolve(WindowHandle);
        if (sdlWindow == null)
        {
            Console.Error.WriteLine($"[ViewportId] RecreateSurface 失败: 无法找到 SDL 窗口 {WindowHandle}");
            return false;
        }

        // 获取原生句柄
        var nativeHandle = sdlWindow.NativeHandle;
        if (nativeHandle == IntPtr.Zero)
        {
            Console.Error.WriteLine($"[ViewportId] RecreateSurface 失败: 原生句柄无效");
            return false;
        }

        // 确定句柄类型
        uint handleType = GetHandleType(sdlWindow);
        if (handleType == 0)
        {
            Console.Error.WriteLine($"[ViewportId] RecreateSurface 失败: 不支持的句柄类型");
            return false;
        }

        // 重建渲染表面
        var success = surfaceRegistry.RecreateSurface(SurfaceId, nativeHandle, handleType);
        Console.WriteLine($"[ViewportId] RecreateSurface: {this}, success={success}");
        return success;
    }

    /// <summary>
    /// 标记 Deferred Resize。
    ///
    /// 调用时机：Avalonia 布局变化时。
    /// </summary>
    /// <param name="surfaceRegistry">渲染表面注册表。</param>
    /// <param name="width">新宽度。</param>
    /// <param name="height">新高度。</param>
    public void MarkResize(IViewportSurfaceRegistry surfaceRegistry, uint width, uint height)
    {
        if (!IsRegistered)
        {
            return;
        }

        surfaceRegistry.MarkResize(SurfaceId, width, height);
    }

    /// <summary>
    /// 帧末统一执行所有 Deferred Resize。
    /// </summary>
    /// <param name="surfaceRegistry">渲染表面注册表。</param>
    public void FlushResizes(IViewportSurfaceRegistry surfaceRegistry)
    {
        surfaceRegistry.FlushResizes();
    }

    /// <summary>
    /// 标记表面丢失。
    ///
    /// 调用时机：原生句柄销毁时。
    /// </summary>
    /// <param name="surfaceRegistry">渲染表面注册表。</param>
    public void MarkSurfaceLost(IViewportSurfaceRegistry surfaceRegistry)
    {
        if (!IsRegistered)
        {
            return;
        }

        surfaceRegistry.MarkSurfaceLost(SurfaceId);
        Console.WriteLine($"[ViewportId] MarkSurfaceLost: {this}");
    }

    /// <summary>
    /// 渲染视口。
    ///
    /// 调用时机：每帧渲染时。
    /// </summary>
    /// <param name="surfaceRegistry">渲染表面注册表。</param>
    /// <param name="width">视口宽度。</param>
    /// <param name="height">视口高度。</param>
    /// <returns>是否渲染成功。</returns>
    public bool RenderViewport(IViewportSurfaceRegistry surfaceRegistry, float width, float height)
    {
        if (!IsRegistered || ViewportService == null)
        {
            return false;
        }

        return surfaceRegistry.RenderViewport(SurfaceId, width, height, ViewportService);
    }

    /// <summary>
    /// Present SwapChain。
    /// </summary>
    /// <param name="surfaceRegistry">渲染表面注册表。</param>
    public void Present(IViewportSurfaceRegistry surfaceRegistry)
    {
        if (!IsRegistered)
        {
            return;
        }

        surfaceRegistry.Present(SurfaceId);
    }

    /// <summary>
    /// 注销渲染表面。
    ///
    /// 调用时机：窗口销毁时。
    /// </summary>
    /// <param name="surfaceRegistry">渲染表面注册表。</param>
    public void Unregister(IViewportSurfaceRegistry surfaceRegistry)
    {
        if (!IsRegistered)
        {
            return;
        }

        surfaceRegistry.Unregister(SurfaceId);
        SurfaceId = 0;

        // 更新 ViewportIdManager
        ViewportIdManager.Unregister(this);

        Console.WriteLine($"[ViewportId] 注销成功: {this}");
    }

    /// <summary>
    /// 根据 SDL 窗口确定句柄类型。
    /// </summary>
    private static uint GetHandleType(SdlWindow sdlWindow)
    {
        // 通过 SdlWindow 的 NativeHandle 判断平台
        // 这里需要根据实际情况调整
        // 暂时返回 Win32HWND，后续可以扩展
        return (uint)NNNativeHandleType.Win32HWND;
    }

    public bool Equals(ViewportId? other)
    {
        if (other is null) return false;
        if (ReferenceEquals(this, other)) return true;
        return WindowHandle.Equals(other.WindowHandle) && SurfaceId == other.SurfaceId;
    }

    public override bool Equals(object? obj)
    {
        return obj is ViewportId other && Equals(other);
    }

    public override int GetHashCode()
    {
        return HashCode.Combine(WindowHandle, SurfaceId);
    }

    public override string ToString()
    {
        return $"ViewportId(Window={WindowHandle}, Surface={SurfaceId}, Service={ViewportService?.GetType().Name ?? "null"})";
    }

    public static bool operator ==(ViewportId? left, ViewportId? right)
    {
        if (left is null) return right is null;
        return left.Equals(right);
    }

    public static bool operator !=(ViewportId? left, ViewportId? right)
    {
        return !(left == right);
    }
}
