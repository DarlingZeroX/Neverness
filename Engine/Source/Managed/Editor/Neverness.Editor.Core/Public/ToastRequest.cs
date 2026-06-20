namespace Neverness.Editor.Core.Public;

/// <summary>Toast 通知类型。</summary>
public enum ToastType
{
    Info,
    Success,
    Warning,
    Error
}

/// <summary>Toast 通知请求数据。</summary>
public readonly record struct ToastRequest(
    string Message,
    ToastType Type = ToastType.Info,
    int DurationMs = 2500);
