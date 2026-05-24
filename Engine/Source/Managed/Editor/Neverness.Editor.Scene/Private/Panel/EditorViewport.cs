using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Public;
using System;
using System.Collections.Generic;
using System.Numerics;

namespace Neverness.Editor.Scene.Private.Panel;

public class EditorViewport : IEditorPanel
{
    private bool m_IsOpen = true;

    public EditorViewport()
    {
    }

    public void OnUpdate(float delta)
    {
        throw new NotImplementedException();
    }

    public void OnFixedUpdate()
    {
        throw new NotImplementedException();
    }

    public bool IsAsync()
    {
        throw new NotImplementedException();
    }

    public void OnGUI()
    {
        if (!m_IsOpen)
            return;

        // Hexa.NET 中的 Begin 使用 string 即可
        if (ImGui.Begin(GetWindowFullName(), ImGuiWindowFlags.NoScrollbar))
        {

        }

        ImGui.End();
    }

    public string GetWindowFullName()
    {
        return FontAwesome5Pro.Window + " " + GetWindowName();
    }

    public string GetWindowName() => "Main Viewport";

    public void OpenWindow(bool open) => m_IsOpen = open;

    public bool IsWindowOpened() => m_IsOpen;
}
