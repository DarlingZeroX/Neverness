namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// ImGui 窗口管理器接口，负责窗口的创建、查找、生命周期管理和逐帧渲染。
/// </summary>
public interface IImWindowManager
{
    // ── 生命周期 ──

    /// <summary>打开一个窗口（非 MultiInstance 类型复用已有实例）。</summary>
    T OpenWindow<T>() where T : ImWindow, new();

    /// <summary>打开一个窗口并执行配置回调。</summary>
    T OpenWindow<T>(Action<T>? configure) where T : ImWindow, new();

    /// <summary>通过类型打开窗口（需先注册工厂）。</summary>
    ImWindow OpenWindow(Type windowType);

    /// <summary>关闭指定窗口。</summary>
    void CloseWindow(Guid windowId);

    /// <summary>关闭所有窗口。</summary>
    void CloseAll();

    /// <summary>聚焦指定窗口（设为可见）。</summary>
    void FocusWindow(Guid windowId);

    // ── 查询 ──

    /// <summary>查找第一个指定类型的窗口。</summary>
    T? FindWindow<T>() where T : ImWindow;

    /// <summary>按 Guid 查找窗口。</summary>
    ImWindow? FindWindow(Guid windowId);

    /// <summary>获取所有窗口快照。</summary>
    IReadOnlyList<ImWindow> GetAllWindows();

    /// <summary>查找所有指定类型的窗口。</summary>
    IReadOnlyList<T> FindWindows<T>() where T : ImWindow;

    /// <summary>指定类型窗口是否处于打开状态。</summary>
    bool IsWindowOpen<T>() where T : ImWindow;

    // ── 帧循环 ──

    /// <summary>逐帧渲染所有可见窗口。</summary>
    void RenderAllWindows();

    /// <summary>逐帧更新所有窗口逻辑。</summary>
    void Update(float deltaTime);

    // ── 注册 ──

    /// <summary>注册窗口类型工厂（泛型）。</summary>
    void RegisterType<T>() where T : ImWindow, new();

    /// <summary>注册窗口类型工厂（Type）。</summary>
    void RegisterType(Type windowType);

    /// <summary>注销窗口类型工厂。</summary>
    void UnregisterType<T>() where T : ImWindow, new();

    // ── 事件 ──

    /// <summary>窗口打开事件。</summary>
    event Action<ImWindow>? WindowOpened;

    /// <summary>窗口关闭事件。</summary>
    event Action<ImWindow>? WindowClosed;

    /// <summary>窗口聚焦事件。</summary>
    event Action<ImWindow>? WindowFocused;
}
