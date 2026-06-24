using Avalonia.Controls;
using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Views.Editors;

/// <summary>
/// 字段编辑器工厂——根据字段类型创建对应的编辑控件。
/// </summary>
internal static class FieldEditorFactory
{
    /// <summary>根据字段类型创建编辑控件。</summary>
    public static Control? Create(FieldDescriptor field, SettingsTable table)
    {
        return field.FieldType switch
        {
            FieldType.Bool => BoolFieldEditor.Create(field, table),
            FieldType.Int => IntFieldEditor.Create(field, table),
            FieldType.Float => FloatFieldEditor.Create(field, table),
            FieldType.String => StringFieldEditor.Create(field, table),
            FieldType.Enum => EnumFieldEditor.Create(field, table),
            _ => FallbackFieldEditor.Create(field),
        };
    }
}
