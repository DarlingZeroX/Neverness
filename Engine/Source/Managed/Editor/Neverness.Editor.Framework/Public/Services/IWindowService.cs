namespace Neverness.Editor.Framework.Public.Services;

/// <summary>
/// 窗口服务接口——由 Frontend 模块实现。
///
/// 职责：
/// - 创建独立窗口（如 TextureViewer、AudioViewer）
/// - 创建对话框（确认、输入、选择）
/// - 窗口生命周期管理
///
/// 设计原则：
/// - Core 只看接口，不依赖具体 UI 框架
/// - AvaloniaFrontend 和 ImGuiFrontend 各自实现
/// </summary>
public interface IWindowService
{
    /// <summary>创建独立窗口。</summary>
    IEditorWindow CreateWindow(string title, int width, int height);

    /// <summary>显示确认对话框。</summary>
    bool ShowConfirmDialog(string title, string message);

    /// <summary>显示输入对话框。</summary>
    string? ShowInputDialog(string title, string message, string defaultValue = "");

    /// <summary>显示选择对话框。</summary>
    int ShowChoiceDialog(string title, string message, string[] options);

    /// <summary>显示打开文件对话框。</summary>
    string? ShowOpenFileDialog(string title, string filter);

    /// <summary>显示保存文件对话框。</summary>
    string? ShowSaveFileDialog(string title, string filter, string defaultName = "");

    /// <summary>显示选择目录对话框。</summary>
    string? ShowFolderDialog(string title);
}

/// <summary>
/// 编辑器窗口接口。
/// </summary>
public interface IEditorWindow : IDisposable
{
    /// <summary>窗口标题。</summary>
    string Title { get; set; }

    /// <summary>窗口是否可见。</summary>
    bool IsVisible { get; }

    /// <summary>显示窗口。</summary>
    void Show();

    /// <summary>隐藏窗口。</summary>
    void Hide();

    /// <summary>关闭窗口。</summary>
    void Close();

    /// <summary>设置窗口内容。</summary>
    void SetContent(object content);
}
