using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Scene.Private.Panel;

/// <summary>
/// 场景层级面板：跟踪当前选中的场景实体句柄（权威数据在 Native 场景图）。
/// </summary>
public class SceneBrowser : IEditorPanel
{
    private bool m_IsOpen = true;

    public SceneBrowser()
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

    public string GetWindowName() => "SceneBrowser";

    public void OpenWindow(bool open) => m_IsOpen = open;

    public bool IsWindowOpened() => m_IsOpen;
}

