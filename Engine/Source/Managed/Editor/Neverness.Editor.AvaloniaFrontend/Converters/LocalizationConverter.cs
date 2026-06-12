using System.Globalization;
using Avalonia.Data.Converters;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.AvaloniaFrontend.Converters;

/// <summary>
/// 本地化转换器——将文本 Key 转换为本地化文本。
///
/// 使用方式（AXAML）：
///   <TextBlock Text="{Binding Title, Converter={x:Static converters:LocalizationConverter.Instance}}"/>
///
/// 或使用 MarkupExtension：
///   <TextBlock Text="{Binding Title, Converter={x:Static converters:LocalizationConverter.Instance}}"/>
/// </summary>
public class LocalizationConverter : IValueConverter
{
    /// <summary>单例实例。</summary>
    public static readonly LocalizationConverter Instance = new();

    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        if (value is not string key)
            return value;

        // 获取本地化文本
        var localizedText = EditorText.Get(key);

        // 如果有格式化参数
        if (parameter is object[] args)
        {
            return string.Format(localizedText, args);
        }

        return localizedText;
    }

    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        // 不支持反向转换
        return value;
    }
}

/// <summary>
/// 本地化 MarkupExtension——在 AXAML 中直接使用。
///
/// 使用方式：
///   <TextBlock Text="{local:Localize 'Open'}"/>
/// </summary>
public class LocalizeExtension : Avalonia.Markup.Xaml.MarkupExtension
{
    public string Key { get; set; } = "";

    public override object ProvideValue(IServiceProvider serviceProvider)
    {
        return EditorText.Get(Key);
    }
}
