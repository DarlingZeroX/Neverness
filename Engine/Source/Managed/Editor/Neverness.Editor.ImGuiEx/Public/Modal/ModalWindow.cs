using Hexa.NET.ImGui;

namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// Modal 弹窗实现。
///
/// 使用 ImGui.BeginPopupModal 实现模态对话框。
/// 支持自定义内容渲染（ContentRenderer）和确认/取消回调。
///
/// 用法:
/// <code>
/// var confirm = new ModalWindow("Confirm Delete");
/// confirm.ContentRenderer = () => {
///     ImGui.Text("确定删除？");
///     if (ImGui.Button("Yes")) return true;  // 返回 true = 确认
///     return false;
/// };
/// confirm.OnConfirmed = () => DeleteAsset();
/// confirm.Open();
/// // 帧循环中: confirm.Render();
/// </code>
/// </summary>
public sealed class ModalWindow : IModalWindow
{
    private bool m_ShouldOpen;

    /// <inheritdoc />
    public string Id { get; }

    /// <inheritdoc />
    public bool IsOpen { get; private set; }

    /// <summary>
    /// 内容渲染委托。返回 true 表示用户确认（将自动关闭弹窗并触发 OnConfirmed）。
    /// </summary>
    public Func<bool>? ContentRenderer { get; set; }

    /// <summary>用户确认时的回调。</summary>
    public Action? OnConfirmed { get; set; }

    /// <summary>用户取消（点击 X 或外部关闭）时的回调。</summary>
    public Action? OnCancelled { get; set; }

    /// <summary>ImGui 窗口标志位。</summary>
    public ImGuiWindowFlags WindowFlags { get; set; } = ImGuiWindowFlags.AlwaysAutoResize;

    public ModalWindow(string id)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(id);
        Id = id;
    }

    /// <inheritdoc />
    public void Open()
    {
        m_ShouldOpen = true;
    }

    /// <inheritdoc />
    public void Close()
    {
        IsOpen = false;
        ImGui.CloseCurrentPopup();
    }

    /// <inheritdoc />
    public bool Render()
    {
        // 延迟打开：ImGui 要求 OpenPopup 在 BeginPopupModal 之前调用
        if (m_ShouldOpen)
        {
            ImGui.OpenPopup(Id);
            m_ShouldOpen = false;
            IsOpen = true;
        }

        bool open = true;
        if (ImGui.BeginPopupModal(Id, ref open, WindowFlags))
        {
            bool confirmed = false;

            if (ContentRenderer != null)
            {
                confirmed = ContentRenderer();
            }

            if (confirmed)
            {
                Close();
                OnConfirmed?.Invoke();
            }

            ImGui.EndPopup();
            return true;
        }

        // 用户点击 X 关闭
        if (!open && IsOpen)
        {
            IsOpen = false;
            OnCancelled?.Invoke();
        }

        return false;
    }
}
