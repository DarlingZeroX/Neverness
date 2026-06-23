using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.ImGuiFrontend.Menu;
using Neverness.Runtime.Application;
using Neverness.Runtime.Application.Private;
using Neverness.Runtime.Application.Public;

namespace Neverness.Editor.ImGuiFrontend.Shell;

/// <summary>
/// 编辑器菜单栏——渲染应用标题栏、菜单项和窗口控制按钮。
/// </summary>
public class ImGuiMenuBar : IEditorPanel
{
    private bool m_Dragging;
    private bool m_EditorMaximized;
    private SdlWindow m_EditorWindow;

    private bool m_ShowImGuiDemo;
    private static (int X, int Y) s_WindowSize = (0, 0);
    private static (int X, int Y) s_WindowPos = (0, 0);

    public ImGuiMenuBar(SdlWindow window)
    {
        m_EditorWindow = window;
    }

    public bool IsAsync()
    {
        return false;
    }

    public void OnGUI()
    {
        var style = ImGui.GetStyle();
        var borderSize = style.WindowBorderSize;
        style.WindowBorderSize = 0.0f;

        ImGui.PushStyleVar(ImGuiStyleVar.FramePadding, new Vector2(4, 7));
        ImGui.PushStyleVar(ImGuiStyleVar.ItemSpacing, new Vector2(10, 4));

        // 使用菜单框架渲染主菜单栏
        var tree = MenuRegistryImp.Instance.GetTree();
        var renderer = ImGuiMenuRendererWrapper.Instance;
        if (renderer.BeginMainMenuBar())
        {
            HandleDraggingWindow();
            renderer.RenderMenuItems(tree);
            HandleWindowControl();
            renderer.EndMainMenuBar();
        }

        ImGui.PopStyleVar(2);
        style.WindowBorderSize = borderSize;

        if (m_ShowImGuiDemo)
        {
            ImGui.ShowDemoWindow(ref m_ShowImGuiDemo);
        }
    }

    // ================= UI LOGIC PLACEHOLDERS =================

    private void HandleDraggingWindow()
    {
        // UI structure only (no real window movement)
        if (!ImGui.IsWindowHovered() && !m_Dragging)
            return;

        if (!ImGui.IsMouseDragging(ImGuiMouseButton.Left))
        {
            m_Dragging = false;
            return;
        }

        if (m_Dragging == false)
        {
            s_WindowPos = m_EditorWindow.Position;
        }

        m_Dragging = true;

        if (m_EditorMaximized == false)
        {
            // 获取鼠标拖拽增量
            Vector2 dragDelta = ImGui.GetMouseDragDelta(ImGuiMouseButton.Left);

            int x = (int)(s_WindowPos.X + dragDelta.X);
            int y = (int)(s_WindowPos.Y + dragDelta.Y);

            // 不允许拖出屏幕顶部
            y = Math.Max(y, 0);

            m_EditorWindow.Position = (x, y);
        }

        m_Dragging = true;
    }

    private void HandleWindowControl()
    {
        ImGui.Indent(ImGui.GetWindowWidth() - 160);

        ImGui.PushID("MainMenuBar_WindowControl");

        ImGui.PushStyleColor(ImGuiCol.Button, new Vector4(0.1f, 0.1f, 0.1f, 0.1f));
        ImGui.PushStyleColor(ImGuiCol.ButtonHovered, new Vector4(1, 1, 1, 0.2f));
        ImGui.PushStyleColor(ImGuiCol.ButtonActive, new Vector4(1, 1, 1, 0.4f));
        ImGui.PushStyleColor(ImGuiCol.Border, new Vector4(0, 0, 0, 0.01f));

        var size = new Vector2(40, ImGui.GetFrameHeight());

        if (ImGui.Button("─" + "##MinEditor", size))
        {
            m_EditorWindow.Minimize();
        }

        if (m_EditorMaximized)
        {
            if (ImGui.Button("❐" + "##RestoreEditor", size))
            {
                m_EditorWindow.Restore();
                m_EditorMaximized = false;
            }
        }
        else
        {
            if (ImGui.Button("□" + "##MaximizeEditor", size))
            {
                m_EditorWindow.Maximize();
                m_EditorMaximized = true;
            }
        }
        if (ImGui.Button("✕" + "##close", size))
        {
           ApplicationHost.Shutdown();
        }

        ImGui.PopStyleColor(4);
        ImGui.PopID();
    }

    // ================= INTERFACE STUB =================

    public void OnUpdate(float delta) { }
    public void OnFixedUpdate() { }

    public string GetWindowFullName() => "EditorMenuBar";
    public string GetWindowName() => "EditorMenuBar";

    public void OpenWindow(bool open) { }

    public bool IsWindowOpened() => true;
}
