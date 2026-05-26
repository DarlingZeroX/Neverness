namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// Modal 弹窗接口。
/// </summary>
public interface IModalWindow
{
    /// <summary>弹窗唯一 ID（用于 ImGui.OpenPopup / BeginPopupModal）。</summary>
    string Id { get; }

    /// <summary>弹窗当前是否处于打开状态。</summary>
    bool IsOpen { get; }

    /// <summary>请求打开弹窗（下一帧生效）。</summary>
    void Open();

    /// <summary>关闭当前弹窗。</summary>
    void Close();

    /// <summary>逐帧渲染弹窗。返回 true 表示弹窗当前正在显示。</summary>
    bool Render();
}
