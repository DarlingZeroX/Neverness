using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Threading;

namespace Neverness.Editor.AvaloniaFrontend.Services;

/// <summary>Toast 通知类型。</summary>
public enum ToastType
{
    Info,
    Success,
    Warning,
    Error
}

/// <summary>
/// 简单的 Toast 通知服务——V1 实现。
///
/// 显示在 Content Browser 右下角，2-3 秒后自动消失。
///
/// 使用方式：
///   ToastService.Instance.SetContainer(overlayPanel);
///   ToastService.Instance.Show("导入成功", ToastType.Success);
/// </summary>
public class ToastService
{
    private static ToastService? _instance;
    public static ToastService Instance => _instance ??= new ToastService();

    private readonly Queue<ToastMessage> _queue = new();
    private Panel? _container;
    private bool _isShowing;

    private ToastService() { }

    /// <summary>设置 Toast 容器（通常是主窗口的 Overlay 层）。</summary>
    public void SetContainer(Panel container)
    {
        _container = container;
    }

    /// <summary>显示 Toast 消息。</summary>
    public void Show(string message, ToastType type = ToastType.Info, int durationMs = 2500)
    {
        if (_container == null)
        {
            // 如果没有容器，只输出到控制台
            Console.WriteLine($"[Toast] {type}: {message}");
            return;
        }

        var toast = new ToastMessage(message, type, durationMs);
        _queue.Enqueue(toast);

        if (!_isShowing)
        {
            Dispatcher.UIThread.Post(ShowNext);
        }
    }

    private void ShowNext()
    {
        if (_queue.Count == 0 || _container == null)
        {
            _isShowing = false;
            return;
        }

        _isShowing = true;
        var toast = _queue.Dequeue();

        // 创建 Toast UI
        var border = CreateToastBorder(toast);

        // 添加到容器
        _container.Children.Add(border);

        // 定位到右下角
        Canvas.SetRight(border, 16);
        Canvas.SetBottom(border, 16);

        // 自动消失
        Task.Delay(toast.DurationMs).ContinueWith(_ =>
        {
            Dispatcher.UIThread.Post(() =>
            {
                _container.Children.Remove(border);
                ShowNext();
            });
        });
    }

    private Border CreateToastBorder(ToastMessage toast)
    {
        var (bgColor, iconColor, icon) = toast.Type switch
        {
            ToastType.Success => ("#FF2D7D2D", "#FF4CAF50", "✓"),
            ToastType.Warning => ("#FFFF9800", "#FFFF9800", "⚠"),
            ToastType.Error => ("#FFD32F2F", "#FFF44336", "✗"),
            _ => ("#FF1976D2", "#FF2196F3", "ℹ"),
        };

        return new Border
        {
            Background = new SolidColorBrush(Color.Parse(bgColor)),
            CornerRadius = new CornerRadius(4),
            Padding = new Thickness(12, 8),
            MaxWidth = 300,
            BoxShadow = new BoxShadows(new BoxShadow
            {
                Color = Color.FromArgb(0x40, 0x00, 0x00, 0x00),
                Blur = 8,
                OffsetX = 0,
                OffsetY = 2,
            }),
            Child = new StackPanel
            {
                Orientation = Orientation.Horizontal,
                Spacing = 8,
                Children =
                {
                    new TextBlock
                    {
                        Text = icon,
                        FontSize = 14,
                        Foreground = new SolidColorBrush(Color.Parse(iconColor)),
                        VerticalAlignment = VerticalAlignment.Center,
                    },
                    new TextBlock
                    {
                        Text = toast.Message,
                        FontSize = 12,
                        Foreground = new SolidColorBrush(Color.Parse("#FFFFFFFF")),
                        TextTrimming = TextTrimming.CharacterEllipsis,
                        MaxLines = 2,
                        VerticalAlignment = VerticalAlignment.Center,
                    }
                }
            }
        };
    }

    private record ToastMessage(string Message, ToastType Type, int DurationMs);
}
