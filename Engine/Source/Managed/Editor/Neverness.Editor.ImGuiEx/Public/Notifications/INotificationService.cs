namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// Toast 通知服务接口。
///
/// 支持 Info/Success/Warning/Error 四个级别，自动消失，右下角显示。
/// </summary>
public interface INotificationService
{
    /// <summary>显示一条通知，返回通知 ID（可用于 Dismiss）。</summary>
    Guid Show(NotificationLevel level, string title, string message, TimeSpan? duration = null);

    /// <summary>显示 Info 通知。</summary>
    Guid Info(string title, string message);

    /// <summary>显示 Success 通知。</summary>
    Guid Success(string title, string message);

    /// <summary>显示 Warning 通知。</summary>
    Guid Warning(string title, string message);

    /// <summary>显示 Error 通知（默认持续 8 秒）。</summary>
    Guid Error(string title, string message);

    /// <summary>手动关闭指定通知。</summary>
    void Dismiss(Guid id);

    /// <summary>关闭所有通知。</summary>
    void DismissAll();

    /// <summary>逐帧更新（处理过期自动消失）。在 Render 之前调用。</summary>
    void Update(float deltaTime);

    /// <summary>逐帧渲染 Toast 叠加层。应在所有窗口渲染之后调用。</summary>
    void Render();
}
