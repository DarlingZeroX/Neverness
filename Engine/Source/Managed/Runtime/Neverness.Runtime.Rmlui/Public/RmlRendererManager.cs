using RmlUiNet;
using RmlUiNet.Input;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 渲染器管理器——单例，按 surfaceId 管理多个 RmlRenderer 实例。
///
/// 职责：
/// - 管理 RmlUI 全局初始化（Rml.Initialise / Rml.Shutdown 只调一次）
/// - 创建/销毁/查询 surfaceId → RmlRenderer 映射
/// - 批量更新、热重载通知
///
/// 设计：
/// - 单例模式，通过 RmlRendererManager.Instance 访问
/// - 每个 surfaceId 对应独立的 RmlRenderer（独立 C++ Handle + RmlUi Context + 文档管理器）
/// - RmlRendererManager 拥有全局初始化，Renderer 通过 skipGlobalInit 跳过重复初始化
///
/// 使用方法：
///   RmlRendererManager.Instance.Initialize();
///   var renderer = RmlRendererManager.Instance.GetOrCreateRenderer(surfaceId);
///   renderer.DocumentManager.LoadDocument(entity, "/ui/main.rml");
///   // 每帧
///   RmlRendererManager.Instance.UpdateAll(deltaTime);
///   // 清理
///   RmlRendererManager.Instance.Shutdown();
/// </summary>
public sealed class RmlRendererManager : IDisposable
{
    #region 单例

    /// <summary>全局单例实例。</summary>
    private static RmlRendererManager? _instance;

    /// <summary>获取全局单例实例（首次访问时自动创建）。</summary>
    public static RmlRendererManager Instance => _instance ??= new RmlRendererManager();

    /// <summary>私有构造函数，防止外部实例化。</summary>
    private RmlRendererManager()
    {
    }

    #endregion

    /// <summary>surfaceId → RmlRenderer 映射。</summary>
    private readonly Dictionary<ulong, RmlRenderer> _renderers = new();

    /// <summary>全局初始化状态。</summary>
    //private bool _globalInitialized;

    /// <summary>是否已释放。</summary>
    private bool _disposed;

    #region 属性

    /// <summary>已注册的渲染器数量。</summary>
    public int Count => _renderers.Count;

    /// <summary>所有已注册的 surfaceId。</summary>
    public IEnumerable<ulong> SurfaceIds => _renderers.Keys;

    /// <summary>全局是否已初始化。</summary>
    //public bool IsInitialized => _globalInitialized;

    #endregion

    #region 全局生命周期

    /// <summary>
    /// 初始化 RmlUI 全局系统。
    ///
    /// 必须在创建任何渲染器之前调用。
    /// 内部调用 Rml.Initialise()，整个进程只调一次。
    /// </summary>
    //public void Initialize()
    //{
    //    if (_globalInitialized)
    //    {
    //        Console.WriteLine("[RmlRendererManager] Already initialized");
    //        return;
    //    }
    //
    //    Rml.Initialise();
    //    _globalInitialized = true;
    //    Console.WriteLine("[RmlRendererManager] Global RmlUI initialized");
    //}

    /// <summary>
    /// 关闭 RmlUI 全局系统。
    ///
    /// 释放所有渲染器，然后调用 Rml.Shutdown()。
    /// </summary>
    public void Shutdown()
    {
        if (_disposed) return;

        // 释放所有渲染器
        foreach (var (surfaceId, renderer) in _renderers)
        {
            renderer.Dispose();
            Console.WriteLine($"[RmlRendererManager] Disposed renderer for surfaceId={surfaceId}");
        }
        _renderers.Clear();

        // 全局关闭
        //if (_globalInitialized)
        //{
        //    Rml.Shutdown();
        //    _globalInitialized = false;
        //    Console.WriteLine("[RmlRendererManager] Global RmlUI shut down");
        //}

        _disposed = true;
    }

    #endregion

    #region 渲染器管理

    /// <summary>
    /// 为指定 surfaceId 创建渲染器。
    ///
    /// 每个 surfaceId 只能有一个渲染器。重复创建返回已有实例。
    /// </summary>
    /// <param name="surfaceId">视口表面 ID。</param>
    /// <param name="width">视口宽度。</param>
    /// <param name="height">视口高度。</param>
    /// <returns>创建或已存在的渲染器。</returns>
    /// <exception cref="InvalidOperationException">全局未初始化。</exception>
    public RmlRenderer CreateRenderer(ulong surfaceId, int width, int height)
    {
        ObjectDisposedException.ThrowIf(_disposed, this);

        //if (!_globalInitialized)
        //    throw new InvalidOperationException("[RmlRendererManager] Must call Initialize() before creating renderers");

        // 已存在则直接返回
        if (_renderers.TryGetValue(surfaceId, out var existing))
        {
            Console.WriteLine($"[RmlRendererManager] Renderer already exists for surfaceId={surfaceId}");
            return existing;
        }

        // 创建新渲染器（跳过全局初始化，由 Manager 统一管理）
        var renderer = new RmlRenderer(width, height, skipGlobalInit: true);
        _renderers[surfaceId] = renderer;

        Console.WriteLine($"[RmlRendererManager] Created renderer for surfaceId={surfaceId}, {width}x{height}");
        return renderer;
    }

    /// <summary>
    /// 获取指定 surfaceId 的渲染器，不存在则自动创建（默认 1280x720）。
    /// </summary>
    /// <param name="surfaceId">视口表面 ID。</param>
    /// <returns>渲染器实例（已存在或新建）。</returns>
    public RmlRenderer GetOrCreateRenderer(ulong surfaceId)
    {
        return GetRenderer(surfaceId) ?? CreateRenderer(surfaceId, 1280, 720);
    }

    /// <summary>
    /// 获取指定 surfaceId 的渲染器。
    /// </summary>
    /// <param name="surfaceId">视口表面 ID。</param>
    /// <returns>渲染器实例，未找到返回 null。</returns>
    public RmlRenderer? GetRenderer(ulong surfaceId)
    {
        _renderers.TryGetValue(surfaceId, out var renderer);
        return renderer;
    }

    /// <summary>
    /// 销毁指定 surfaceId 的渲染器。
    /// </summary>
    /// <param name="surfaceId">视口表面 ID。</param>
    /// <returns>是否成功销毁。</returns>
    public bool DestroyRenderer(ulong surfaceId)
    {
        if (!_renderers.TryGetValue(surfaceId, out var renderer))
            return false;

        renderer.Dispose();
        _renderers.Remove(surfaceId);

        Console.WriteLine($"[RmlRendererManager] Destroyed renderer for surfaceId={surfaceId}");
        return true;
    }

    /// <summary>
    /// 检查指定 surfaceId 是否有渲染器。
    /// </summary>
    /// <param name="surfaceId">视口表面 ID。</param>
    /// <returns>是否存在。</returns>
    public bool HasRenderer(ulong surfaceId)
    {
        return _renderers.ContainsKey(surfaceId);
    }

    #endregion

    #region 批量操作

    /// <summary>
    /// 更新所有渲染器。
    /// </summary>
    /// <param name="deltaTime">帧间隔时间。</param>
    public void UpdateAll(float deltaTime)
    {
        foreach (var renderer in _renderers.Values)
        {
            renderer.Update(deltaTime);
        }
    }

    /// <summary>
    /// 文件变化通知——广播到所有渲染器的文档管理器。
    /// </summary>
    /// <param name="filePath">变化的文件路径。</param>
    public void NotifyFileChanged(string filePath)
    {
        foreach (var renderer in _renderers.Values)
        {
            renderer.DocumentManager.NotifyFileChanged(filePath);
        }
    }

    /// <summary>
    /// 调整指定 surfaceId 的视口尺寸。
    /// </summary>
    /// <param name="surfaceId">视口表面 ID。</param>
    /// <param name="width">新宽度。</param>
    /// <param name="height">新高度。</param>
    /// <returns>是否成功。</returns>
    public bool Resize(ulong surfaceId, int width, int height)
    {
        if (!_renderers.TryGetValue(surfaceId, out var renderer))
            return false;

        renderer.Resize(width, height);
        return true;
    }

    #endregion

    #region 输入路由（按 surfaceId 分发）

    /// <summary>处理鼠标移动。</summary>
    public bool OnMouseMove(ulong surfaceId, int x, int y, KeyModifier modifiers)
    {
        return _renderers.TryGetValue(surfaceId, out var renderer) && renderer.OnMouseMove(x, y, modifiers);
    }

    /// <summary>处理鼠标按键。</summary>
    public bool OnMouseButton(ulong surfaceId, MouseButton button, bool down, KeyModifier modifiers)
    {
        return _renderers.TryGetValue(surfaceId, out var renderer) && renderer.OnMouseButton(button, down, modifiers);
    }

    /// <summary>处理鼠标滚轮。</summary>
    public bool OnMouseWheel(ulong surfaceId, float delta, KeyModifier modifiers)
    {
        return _renderers.TryGetValue(surfaceId, out var renderer) && renderer.OnMouseWheel(delta, modifiers);
    }

    /// <summary>处理键盘按键。</summary>
    public bool OnKey(ulong surfaceId, KeyIdentifier key, bool down, KeyModifier modifiers, float nativeDpRatio = 1.0f)
    {
        return _renderers.TryGetValue(surfaceId, out var renderer) && renderer.OnKey(key, down, modifiers, nativeDpRatio);
    }

    /// <summary>处理文本输入。</summary>
    public bool OnTextInput(ulong surfaceId, string text)
    {
        return _renderers.TryGetValue(surfaceId, out var renderer) && renderer.OnTextInput(text);
    }

    /// <summary>处理鼠标离开。</summary>
    public void OnMouseLeave(ulong surfaceId)
    {
        if (_renderers.TryGetValue(surfaceId, out var renderer))
            renderer.OnMouseLeave();
    }

    #endregion

    #region IDisposable

    public void Dispose()
    {
        Shutdown();
    }

    #endregion
}
