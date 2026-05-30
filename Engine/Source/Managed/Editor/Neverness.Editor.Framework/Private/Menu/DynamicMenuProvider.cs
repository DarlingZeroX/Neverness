using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Framework.Private.Menu;

/// <summary>
/// 动态菜单项构建器——供应器回调中使用，用于运行时生成子菜单项。
/// </summary>
public sealed class DynamicMenuBuilder
{
    private readonly List<EditorMenuItem> _items = [];

    /// <summary>添加动态菜单项。</summary>
    /// <param name="label">显示标签。</param>
    /// <param name="execute">点击回调。</param>
    /// <param name="icon">图标（FontAwesome5Pro 常量）。</param>
    /// <param name="shortcut">快捷键。</param>
    public DynamicMenuBuilder Add(string label, Action execute, string icon = "", string shortcut = "")
    {
        _items.Add(new EditorMenuItem(
            Path: label,
            Command: new EditorCommand
            {
                Id = "dynamic." + label.Replace(" ", "_"),
                DisplayName = label,
                Execute = _ => execute(),
            },
            Icon: icon,
            Shortcut: shortcut
        ));
        return this;
    }

    /// <summary>添加分隔符。</summary>
    public DynamicMenuBuilder Separator()
    {
        _items.Add(new EditorMenuItem(Path: "sep", IsSeparator: true));
        return this;
    }

    /// <summary>构建结果列表（内部使用）。</summary>
    public IReadOnlyList<EditorMenuItem> Build() => _items;
}
