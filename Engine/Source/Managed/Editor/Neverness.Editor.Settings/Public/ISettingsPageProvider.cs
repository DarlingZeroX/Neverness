using Avalonia.Controls;
using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings;

/// <summary>
/// 自定义页面提供者——复杂设置页面实现此接口。
/// 返回手写 AXAML Control 替代自动生成的 AutoSettingsPanel。
///
/// 用法：
/// <code>
/// public sealed class InputSettingsPageProvider : ISettingsPageProvider
/// {
///     public string TableId => "input";
///     public Control CreatePage(SettingsTable table) => new InputSettingsPage((InputSettings)table);
/// }
/// </code>
/// </summary>
public interface ISettingsPageProvider
{
    /// <summary>关联的设置表 ID。</summary>
    string TableId { get; }

    /// <summary>创建自定义页面控件。</summary>
    /// <param name="table">关联的设置表实例。</param>
    /// <returns>自定义页面控件。</returns>
    Control CreatePage(SettingsTable table);
}
