using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.AvaloniaFrontend.Controls;

/// <summary>
/// 本地化文本控件——自动根据当前语言显示文本。
///
/// 使用方式（AXAML）：
///   <controls:LocalizedTextBlock Key="Open"/>
///
/// 或在代码中：
///   var textBlock = new LocalizedTextBlock { Key = "Open" };
/// </summary>
public class LocalizedTextBlock : TextBlock
{
    /// <summary>文本 Key 属性。</summary>
    public static readonly StyledProperty<string> KeyProperty =
        AvaloniaProperty.Register<LocalizedTextBlock, string>(nameof(Key));

    /// <summary>文本 Key。</summary>
    public string Key
    {
        get => GetValue(KeyProperty);
        set => SetValue(KeyProperty, value);
    }

    public LocalizedTextBlock()
    {
        // 监听 Key 属性变更
        KeyProperty.Changed.AddClassHandler<LocalizedTextBlock>((x, e) => x.OnKeyChanged());
    }

    protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
    {
        base.OnAttachedToVisualTree(e);
        UpdateText();
    }

    private void OnKeyChanged()
    {
        UpdateText();
    }

    private void UpdateText()
    {
        if (string.IsNullOrEmpty(Key))
        {
            Text = "";
            return;
        }

        Text = EditorText.Get(Key);
    }
}
