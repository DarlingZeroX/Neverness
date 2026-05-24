using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Core.Private;

/// <summary>
/// 内置菜单贡献：File / Edit / Window / Help。
/// </summary>
public sealed class BuiltinMenuContributor : IMenuContributor
{
    public void Build(MenuRegistryImp registry)
    {
        // File 菜单
        registry.Register(new EditorMenuItem("File/New Scene", SortOrder: 100, Icon: FontAwesome5Pro.File));
        registry.Register(new EditorMenuItem("File/Open Scene", SortOrder: 200, Icon: FontAwesome5Pro.FolderOpen));
        registry.Register(new EditorMenuItem("File/Save Scene", SortOrder: 300, Icon: FontAwesome5Pro.Save, Shortcut: "Ctrl+S"));
        registry.Register(new EditorMenuItem("File/Save Scene As...", SortOrder: 400));
        registry.Register(new EditorMenuItem("File/sep1", IsSeparator: true, SortOrder: 500));
        registry.Register(new EditorMenuItem("File/Build Settings...", SortOrder: 600, Icon: FontAwesome5Pro.Cog));
        registry.Register(new EditorMenuItem("File/sep2", IsSeparator: true, SortOrder: 700));
        registry.Register(new EditorMenuItem("File/Exit", SortOrder: 900, Icon: FontAwesome5Pro.Times));

        // Edit 菜单
        registry.Register(new EditorMenuItem("Edit/Undo", SortOrder: 100, Icon: FontAwesome5Pro.Undo, Shortcut: "Ctrl+Z"));
        registry.Register(new EditorMenuItem("Edit/Redo", SortOrder: 200, Icon: FontAwesome5Pro.Redo, Shortcut: "Ctrl+Y"));
        registry.Register(new EditorMenuItem("Edit/sep1", IsSeparator: true, SortOrder: 300));
        registry.Register(new EditorMenuItem("Edit/Cut", SortOrder: 400, Icon: FontAwesome5Pro.Cut, Shortcut: "Ctrl+X"));
        registry.Register(new EditorMenuItem("Edit/Copy", SortOrder: 500, Icon: FontAwesome5Pro.Copy, Shortcut: "Ctrl+C"));
        registry.Register(new EditorMenuItem("Edit/Paste", SortOrder: 600, Icon: FontAwesome5Pro.Paste, Shortcut: "Ctrl+V"));
        registry.Register(new EditorMenuItem("Edit/sep2", IsSeparator: true, SortOrder: 700));
        registry.Register(new EditorMenuItem("Edit/Project Settings...", SortOrder: 800, Icon: FontAwesome5Pro.Cog));
        registry.Register(new EditorMenuItem("Edit/Preferences...", SortOrder: 900, Icon: FontAwesome5Pro.SlidersH));

        // Window 菜单
        registry.Register(new EditorMenuItem("Window/Rendering", SortOrder: 100));
        registry.Register(new EditorMenuItem("Window/Analysis", SortOrder: 200));

        // Help 菜单
        registry.Register(new EditorMenuItem("Help/Engine Homepage", SortOrder: 100, Icon: FontAwesome5Pro.Globe));
        registry.Register(new EditorMenuItem("Help/GitHub", SortOrder: 200, Icon: FontAwesome5Pro.CodeBranch));
        registry.Register(new EditorMenuItem("Help/sep1", IsSeparator: true, SortOrder: 300));
        registry.Register(new EditorMenuItem("Help/About Neverness", SortOrder: 900, Icon: FontAwesome5Pro.InfoCircle));
    }
}
