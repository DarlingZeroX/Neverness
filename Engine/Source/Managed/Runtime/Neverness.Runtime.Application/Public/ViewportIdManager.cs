// Neverness.Runtime.Application — 视口标识符管理器。
// 管理所有 ViewportId 实例，提供查找和路由功能。

using Neverness.Rendering.Core;

namespace Neverness.Runtime.Application.Public;

/// <summary>
/// 视口标识符管理器——管理所有 ViewportId 实例。
///
/// 职责：
/// - 注册/注销 ViewportId
/// - 通过 WindowHandle 查找 ViewportId（用于 SDL 事件路由）
/// - 通过 SurfaceId 查找 ViewportId（用于渲染调用）
/// - 在 ApplicationHost.Shutdown() 时清理所有实例
///
/// 使用场景：
/// - SDL 输入事件 → 通过 WindowHandle 找到 ViewportId → 调用 IViewportService
/// - 渲染调用 → 通过 SurfaceId 找到 ViewportId → 调用 IViewportService
/// </summary>
public static class ViewportIdManager
{
    private static readonly Dictionary<WindowHandle, ViewportId> s_byWindowHandle = new();
    private static readonly Dictionary<ulong, ViewportId> s_bySurfaceId = new();

    /// <summary>当前已注册的视口数量。</summary>
    public static int Count => s_byWindowHandle.Count;

    /// <summary>
    /// 注册 ViewportId。
    /// </summary>
    /// <param name="viewportId">要注册的视口标识符。</param>
    /// <returns>是否成功注册。</returns>
    public static bool Register(ViewportId viewportId)
    {
        if (!viewportId.IsValid)
        {
            Console.Error.WriteLine("[ViewportIdManager] 无法注册无效的 ViewportId");
            return false;
        }

        // 检查是否已存在
        if (s_byWindowHandle.ContainsKey(viewportId.WindowHandle))
        {
            Console.Error.WriteLine($"[ViewportIdManager] WindowHandle {viewportId.WindowHandle} 已注册");
            return false;
        }

        if (s_bySurfaceId.ContainsKey(viewportId.SurfaceId))
        {
            Console.Error.WriteLine($"[ViewportIdManager] SurfaceId {viewportId.SurfaceId} 已注册");
            return false;
        }

        // 注册到两个字典
        s_byWindowHandle[viewportId.WindowHandle] = viewportId;
        s_bySurfaceId[viewportId.SurfaceId] = viewportId;

        Console.WriteLine($"[ViewportIdManager] 已注册: {viewportId}");
        return true;
    }

    /// <summary>
    /// 注销 ViewportId。
    /// </summary>
    /// <param name="viewportId">要注销的视口标识符。</param>
    /// <returns>是否成功注销。</returns>
    public static bool Unregister(ViewportId viewportId)
    {
        if (!viewportId.IsValid)
        {
            return false;
        }

        bool removed1 = s_byWindowHandle.Remove(viewportId.WindowHandle);
        bool removed2 = s_bySurfaceId.Remove(viewportId.SurfaceId);

        if (removed1 || removed2)
        {
            Console.WriteLine($"[ViewportIdManager] 已注销: {viewportId}");
            return true;
        }

        return false;
    }

    /// <summary>
    /// 通过 WindowHandle 查找 ViewportId。
    ///
    /// 使用场景：SDL 输入事件路由。
    /// SDL 事件带着 windowId → 转换为 WindowHandle → 查找 ViewportId → 调用 IViewportService
    /// </summary>
    /// <param name="windowHandle">SDL 窗口句柄。</param>
    /// <returns>对应的 ViewportId，未找到返回 Invalid。</returns>
    public static ViewportId FindByWindowHandle(WindowHandle windowHandle)
    {
        return s_byWindowHandle.TryGetValue(windowHandle, out var viewportId) ? viewportId : ViewportId.Invalid;
    }

    /// <summary>
    /// 通过 SDL_WindowID 查找 ViewportId。
    ///
    /// 使用场景：SDL 事件路由（SDL 事件带着 uint32 windowID）。
    /// </summary>
    /// <param name="sdlWindowId">SDL_WindowID。</param>
    /// <returns>对应的 ViewportId，未找到返回 Invalid。</returns>
    public static ViewportId FindBySdlWindowId(uint sdlWindowId)
    {
        return FindByWindowHandle(new WindowHandle(sdlWindowId));
    }

    /// <summary>
    /// 通过 SurfaceId 查找 ViewportId。
    ///
    /// 使用场景：渲染调用。
    /// 渲染系统用 surfaceId → 查找 ViewportId → 调用 IViewportService
    /// </summary>
    /// <param name="surfaceId">渲染表面 ID。</param>
    /// <returns>对应的 ViewportId，未找到返回 Invalid。</returns>
    public static ViewportId FindBySurfaceId(ulong surfaceId)
    {
        return s_bySurfaceId.TryGetValue(surfaceId, out var viewportId) ? viewportId : ViewportId.Invalid;
    }

    /// <summary>
    /// 更新 ViewportId 的 IViewportService。
    ///
    /// 使用场景：Controller 初始化完成后，绑定 IViewportService。
    /// </summary>
    /// <param name="windowHandle">SDL 窗口句柄。</param>
    /// <param name="viewportService">视口服务。</param>
    /// <returns>是否成功更新。</returns>
    public static bool UpdateService(WindowHandle windowHandle, IViewportService viewportService)
    {
        if (!s_byWindowHandle.TryGetValue(windowHandle, out var viewportId))
        {
            Console.Error.WriteLine($"[ViewportIdManager] 未找到 WindowHandle: {windowHandle}");
            return false;
        }

        viewportId.SetViewportService(viewportService);

        Console.WriteLine($"[ViewportIdManager] 已更新 Service: {viewportId}");
        return true;
    }

    /// <summary>
    /// 更新 ViewportId 的 SurfaceId。
    ///
    /// 使用场景：Surface 注册后，绑定 SurfaceId。
    /// 注意：SurfaceId 由 ViewportId.Register() 自动设置，此方法主要用于外部更新。
    /// </summary>
    /// <param name="windowHandle">SDL 窗口句柄。</param>
    /// <param name="surfaceId">渲染表面 ID。</param>
    /// <returns>是否成功更新。</returns>
    public static bool UpdateSurfaceId(WindowHandle windowHandle, ulong surfaceId)
    {
        if (!s_byWindowHandle.TryGetValue(windowHandle, out var viewportId))
        {
            Console.Error.WriteLine($"[ViewportIdManager] 未找到 WindowHandle: {windowHandle}");
            return false;
        }

        // 移除旧的 SurfaceId 映射
        s_bySurfaceId.Remove(viewportId.SurfaceId);

        // 注意：SurfaceId 由 ViewportId.Register() 自动设置
        // 这里只是更新索引
        s_bySurfaceId[surfaceId] = viewportId;

        Console.WriteLine($"[ViewportIdManager] 已更新 SurfaceId 索引: {viewportId}");
        return true;
    }

    /// <summary>
    /// 获取所有 ViewportId。
    /// </summary>
    /// <returns>所有已注册的 ViewportId 列表。</returns>
    public static IEnumerable<ViewportId> GetAll()
    {
        return s_byWindowHandle.Values;
    }

    /// <summary>
    /// 获取游戏视口 ViewportId。
    ///
    /// 返回第一个有效的 ViewportId（游戏主视口）。
    /// 如果无可用视口，返回 ViewportId.Invalid。
    /// </summary>
    /// <returns>游戏视口 ViewportId。</returns>
    public static ViewportId GetGameViewportId()
    {
        return s_byWindowHandle.Values.FirstOrDefault(v => v.IsValid, ViewportId.Invalid);
    }

    /// <summary>
    /// 清理所有 ViewportId。
    ///
    /// 调用时机：ApplicationHost.Shutdown() 时。
    /// </summary>
    public static void Clear()
    {
        s_byWindowHandle.Clear();
        s_bySurfaceId.Clear();
        Console.WriteLine("[ViewportIdManager] 已清理所有 ViewportId");
    }
}
