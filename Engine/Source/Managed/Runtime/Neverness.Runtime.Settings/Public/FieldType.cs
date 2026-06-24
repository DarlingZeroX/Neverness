namespace Neverness.Runtime.Settings;

/// <summary>
/// 设置字段类型枚举。
/// 决定设置 UI 中使用什么控件渲染。
/// </summary>
public enum FieldType
{
    /// <summary>布尔值 → CheckBox。</summary>
    Bool,

    /// <summary>整数 → NumericUpDown（有 Range 时加 Slider）。</summary>
    Int,

    /// <summary>浮点数 → NumericUpDown（有 Range 时加 Slider）。</summary>
    Float,

    /// <summary>字符串 → TextBox。</summary>
    String,

    /// <summary>枚举 → ComboBox。</summary>
    Enum,

    /// <summary>颜色 → ColorPicker（未来扩展）。</summary>
    Color,

    /// <summary>二维向量 → 2x NumericUpDown（未来扩展）。</summary>
    Vector2,

    /// <summary>三维向量 → 3x NumericUpDown（未来扩展）。</summary>
    Vector3,

    /// <summary>文件路径 → TextBox + Browse Button。</summary>
    Path,

    /// <summary>自定义编辑器 → 由 ISettingsPageProvider 处理。</summary>
    Custom
}
