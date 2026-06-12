using Avalonia.Controls;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Public.Mvvm;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// Avalonia View 基类——桥接 IEditorPanel/IView 与 Avalonia 数据绑定。
///
/// 与 ImGui 的 PanelViewBase 对应，但适配 Avalonia 的 retained-mode 模式：
/// - ImGui 版本：OnGUI() 每帧调用 Render() 绘制 UI
/// - Avalonia 版本：OnGUI() 为空操作（UI 由数据绑定自动更新）
///
/// 设计原则：
/// - 继承 UserControl，使其成为 Avalonia 控件
/// - 实现 IEditorPanel 接口（兼容 PanelManager）
/// - 实现 IView 接口（兼容 EditorCompositionRoot）
/// - Bind() 时绑定 ViewModel，后续由数据绑定驱动
/// - Render() 为空操作（Avalonia 不需要每帧绘制）
/// </summary>
public abstract class AvaloniaViewBase : UserControl, IEditorPanel, IView
{
    private readonly string _windowName;
    private readonly string _windowFullName;
    private bool _isOpened = true;

    protected AvaloniaViewBase(string windowName)
        : this(windowName, windowName)
    {
    }

    protected AvaloniaViewBase(string windowName, string windowFullName)
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

    /// <summary>
    /// 由 PanelManager 每帧调用。
    /// Avalonia 版本为空操作——UI 由数据绑定自动更新。
    /// </summary>
    public void OnGUI()
    {
        // Avalonia retained-mode：不需要每帧绘制
        // UI 更新通过 INotifyPropertyChanged 数据绑定驱动
    }

    // ── IView ──

    public abstract Type ViewModelType { get; }

    /// <summary>绑定 ViewModel。子类实现具体的绑定逻辑。</summary>
    public abstract void Bind(object viewModel);

    /// <summary>解绑 ViewModel。</summary>
    public abstract void Unbind();

    /// <summary>
    /// 渲染 UI（每帧调用）。
    /// Avalonia 版本为空操作。
    /// </summary>
    public void Render()
    {
        // Avalonia retained-mode：不需要每帧渲染
    }
}
