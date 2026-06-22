using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Core.Private;

/// <summary>
/// 内置菜单贡献：File / Edit / Window / Help。
/// Scene 相关菜单项由 Scene 模块注册。
/// </summary>
public sealed class BuiltinMenuContributor : IMenuContributor
{
    public void Build(MenuRegistryImp registry)
    {
        // File 菜单（Scene 相关项由 Scene 模块注册）
        registry.Register(new EditorMenuItem("File/sep1", IsSeparator: true, SortOrder: 500));
        registry.Register(new EditorMenuItem("File/Build Settings...", SortOrder: 600, Icon: "⚙️"));
        registry.Register(new EditorMenuItem("File/sep2", IsSeparator: true, SortOrder: 700));
        registry.Register(new EditorMenuItem("File/Exit", SortOrder: 900, Icon: "🚪"));

        // Edit 菜单
        registry.Register(new EditorMenuItem("Edit/Undo", SortOrder: 100, Icon: "↩️", Shortcut: "Ctrl+Z"));
        registry.Register(new EditorMenuItem("Edit/Redo", SortOrder: 200, Icon: "↪️", Shortcut: "Ctrl+Y"));
        registry.Register(new EditorMenuItem("Edit/sep1", IsSeparator: true, SortOrder: 300));
        registry.Register(new EditorMenuItem("Edit/Cut", SortOrder: 400, Icon: "✂️", Shortcut: "Ctrl+X"));
        registry.Register(new EditorMenuItem("Edit/Copy", SortOrder: 500, Icon: "📋", Shortcut: "Ctrl+C"));
        registry.Register(new EditorMenuItem("Edit/Paste", SortOrder: 600, Icon: "📌", Shortcut: "Ctrl+V"));
        registry.Register(new EditorMenuItem("Edit/sep2", IsSeparator: true, SortOrder: 700));
        registry.Register(new EditorMenuItem("Edit/Project Settings...", SortOrder: 800, Icon: "⚙️"));
        registry.Register(new EditorMenuItem("Edit/Preferences...", CommandId: "editor.preferences", SortOrder: 900, Icon: "🔧"));

        // Window 菜单
        registry.Register(new EditorMenuItem("Window/Rendering", SortOrder: 100, Icon: "🖥️"));
        registry.Register(new EditorMenuItem("Window/Analysis", SortOrder: 200, Icon: "📊"));

        // Help 菜单
        registry.Register(new EditorMenuItem("Help/Engine Homepage", SortOrder: 100, Icon: "🌐"));
        registry.Register(new EditorMenuItem("Help/GitHub", SortOrder: 200, Icon: "🐙"));
        registry.Register(new EditorMenuItem("Help/sep1", IsSeparator: true, SortOrder: 300));
        registry.Register(new EditorMenuItem("Help/About Neverness", SortOrder: 900, Icon: "ℹ️"));
    }
}
