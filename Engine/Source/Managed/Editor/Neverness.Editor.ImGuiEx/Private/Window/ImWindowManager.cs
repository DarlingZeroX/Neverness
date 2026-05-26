namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// ImGui 窗口管理器实现。
///
/// 持有所有窗口实例，管理窗口类型工厂，逐帧驱动窗口更新和渲染。
/// 支持单例窗口（默认）和多实例窗口（MultiInstance 标志）。
/// </summary>
public sealed class ImWindowManager : IImWindowManager
{
    // ── 存储 ──

    private readonly Dictionary<Guid, ImWindow> m_Windows = new();
    private readonly Dictionary<Type, Func<ImWindow>> m_Factories = new();

    // ── 事件 ──

    /// <inheritdoc />
    public event Action<ImWindow>? WindowOpened;

    /// <inheritdoc />
    public event Action<ImWindow>? WindowClosed;

    /// <inheritdoc />
    public event Action<ImWindow>? WindowFocused;

    // ── 工厂注册 ──

    /// <inheritdoc />
    public void RegisterType<T>() where T : ImWindow, new()
    {
        m_Factories[typeof(T)] = static () => new T();
    }

    /// <inheritdoc />
    public void RegisterType(Type windowType)
    {
        ArgumentNullException.ThrowIfNull(windowType);

        if (!typeof(ImWindow).IsAssignableFrom(windowType))
            throw new ArgumentException($"类型 {windowType.Name} 不是 ImWindow 的子类。");

        m_Factories[windowType] = () => (ImWindow)Activator.CreateInstance(windowType)!;
    }

    /// <inheritdoc />
    public void UnregisterType<T>() where T : ImWindow, new()
    {
        m_Factories.Remove(typeof(T));
    }

    // ── 打开窗口 ──

    /// <inheritdoc />
    public T OpenWindow<T>() where T : ImWindow, new()
    {
        return OpenWindow<T>(null);
    }

    /// <inheritdoc />
    public T OpenWindow<T>(Action<T>? configure) where T : ImWindow, new()
    {
        // 非 MultiInstance 类型复用已有实例
        var existing = FindWindow<T>();
        if (existing != null && !existing.BehaviorFlags.HasFlag(ImWindowBehaviorFlags.MultiInstance))
        {
            FocusWindow(existing.WindowId);
            return existing;
        }

        var window = new T();
        configure?.Invoke(window);
        RegisterAndOpen(window);
        return window;
    }

    /// <inheritdoc />
    public ImWindow OpenWindow(Type windowType)
    {
        ArgumentNullException.ThrowIfNull(windowType);

        if (!m_Factories.TryGetValue(windowType, out var factory))
            throw new InvalidOperationException(
                $"窗口类型 {windowType.Name} 未注册。请先调用 RegisterType。");

        var window = factory();
        RegisterAndOpen(window);
        return window;
    }

    private void RegisterAndOpen(ImWindow window)
    {
        m_Windows[window.WindowId] = window;
        window.OnOpen();
        WindowOpened?.Invoke(window);
    }

    // ── 关闭窗口 ──

    /// <inheritdoc />
    public void CloseWindow(Guid windowId)
    {
        if (m_Windows.TryGetValue(windowId, out var window))
        {
            window.RequestClose();
            m_Windows.Remove(windowId);
            WindowClosed?.Invoke(window);
        }
    }

    /// <inheritdoc />
    public void CloseAll()
    {
        var ids = m_Windows.Keys.ToList();
        foreach (var id in ids)
        {
            CloseWindow(id);
        }
    }

    // ── 聚焦 ──

    /// <inheritdoc />
    public void FocusWindow(Guid windowId)
    {
        if (m_Windows.TryGetValue(windowId, out var window))
        {
            window.IsVisible = true;
            WindowFocused?.Invoke(window);
        }
    }

    // ── 查询 ──

    /// <inheritdoc />
    public T? FindWindow<T>() where T : ImWindow
    {
        return m_Windows.Values.OfType<T>().FirstOrDefault();
    }

    /// <inheritdoc />
    public ImWindow? FindWindow(Guid windowId)
    {
        return m_Windows.TryGetValue(windowId, out var w) ? w : null;
    }

    /// <inheritdoc />
    public IReadOnlyList<ImWindow> GetAllWindows()
    {
        return m_Windows.Values.ToList();
    }

    /// <inheritdoc />
    public IReadOnlyList<T> FindWindows<T>() where T : ImWindow
    {
        return m_Windows.Values.OfType<T>().ToList();
    }

    /// <inheritdoc />
    public bool IsWindowOpen<T>() where T : ImWindow
    {
        return m_Windows.Values.OfType<T>().Any(w => w.IsOpen);
    }

    // ── 帧循环 ──

    /// <inheritdoc />
    public void RenderAllWindows()
    {
        foreach (var window in m_Windows.Values)
        {
            if (window.IsVisible && window.IsOpen)
            {
                try
                {
                    window.Render();
                }
                catch (Exception ex)
                {
#if DEBUG
                    Console.WriteLine($"[ImWindowManager] 窗口渲染异常: {window.GetType().Name}\n{ex}");
#endif
                }
            }
        }
    }

    /// <inheritdoc />
    public void Update(float deltaTime)
    {
        foreach (var window in m_Windows.Values)
        {
            if (window.IsOpen)
            {
                window.OnUpdate(deltaTime);
            }
        }
    }
}
