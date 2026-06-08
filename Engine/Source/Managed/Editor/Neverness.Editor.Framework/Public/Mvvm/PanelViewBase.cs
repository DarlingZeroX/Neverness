using Neverness.Editor.Framework.Interface;

namespace Neverness.Editor.Framework.Public.Mvvm;

/// <summary>
/// PanelView 基类——桥接旧 IEditorPanel 体系与新 IView 体系。
/// ImGuiFrontend 的 View 继承此类，同时实现 IEditorPanel 和 IView。
///
/// 设计说明：
/// - 继承此类的 View 同时满足旧 PanelManager（IEditorPanel）和新 MVVM（IView）
/// - OnGUI() 委托给 Render()，保持与现有 PanelManager 的兼容性
/// - 未来废弃 IEditorPanel 时，只需让 View 直接实现 IView
/// </summary>
public abstract class PanelViewBase : IEditorPanel, IView
{
    private readonly string _windowName;
    private readonly string _windowFullName;
    private bool _isOpened = true;

    protected PanelViewBase(string windowName)
        : this(windowName, windowName)
    {
    }

    protected PanelViewBase(string windowName, string windowFullName)
    {
        _windowName = windowName;
        _windowFullName = windowFullName;
    }

    // ── IEditorPanel ──

    public string GetWindowFullName() => _windowFullName;
    public string GetWindowName() => _windowName;
    public void OpenWindow(bool open) => _isOpened = open;
    public bool IsWindowOpened() => _isOpened;
    public virtual bool IsAsync() => false;
    public virtual void OnUpdate(float delta) { }
    public virtual void OnFixedUpdate() { }

    /// <summary>由 PanelManager 每帧调用，委托给 Render()。</summary>
    public void OnGUI()
    {
        if (_isOpened)
            Render();
    }

    // ── IView ──

    public abstract Type ViewModelType { get; }
    public abstract void Bind(object viewModel);
    public abstract void Unbind();
    public abstract void Render();
}
