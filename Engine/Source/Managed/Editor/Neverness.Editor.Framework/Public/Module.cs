using Neverness.Runtime.Application.Public;

namespace Neverness.Editor.Framework.Public;
using Neverness.Editor.Framework;
using System;

public static class EditorFrameworkModule
{
    /// <summary>
    /// 将模块挂载到编辑器中。
    /// </summary>
    public static void Install(Window window)
    {
        Private.EditorFrameworkModuleImp.Install(window);
    }

    public static void TickEditorUI()
    {
        Private.EditorFrameworkModuleImp.TickEditorUI();
    }
}
