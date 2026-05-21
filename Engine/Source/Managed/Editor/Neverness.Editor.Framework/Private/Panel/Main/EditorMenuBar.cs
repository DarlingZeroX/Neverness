using System.Numerics;
using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Application;
using Neverness.Runtime.Application.Public;

namespace Neverness.Editor.Framework.Private.Panel.Main;

public class EditorMenuBar : IEditorPanel
{
    private bool m_Dragging;
    private bool m_EditorMaximized;
    //private object? mEditorWindow; // placeholder (VGWindow in C++)
    private Window m_EditorWindow;

    private bool m_ShowImGuiDemo;
    private static (int X, int Y) s_WindowSize = (0, 0);
    private static (int X, int Y) s_WindowPos = (0, 0);

    public EditorMenuBar(Window window)
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

        ImGui.PushStyleVar(ImGuiStyleVar.FramePadding, new System.Numerics.Vector2(4, 7));
        ImGui.PushStyleVar(ImGuiStyleVar.ItemSpacing, new System.Numerics.Vector2(10, 4));

        if (ImGui.BeginMainMenuBar())
        {
            HandleDraggingWindow();

            // ===== Engine Icon (UI only) =====
            ImGui.PushStyleVar(ImGuiStyleVar.FramePadding, new System.Numerics.Vector2(0, 0));
            //ImGui.Image(0, new System.Numerics.Vector2(40, ImGui.GetFrameHeight() * 2)); // texture placeholder
            ImGui.PopStyleVar();

            // ================= FILE =================
            if (ImGui.BeginMenu("File"))
            {
                ImGui.Separator();
                ImGui.MenuItem("New Scene");
                ImGui.MenuItem("Open Scene");
                ImGui.MenuItem("Save Scene");
                ImGui.MenuItem("Save Scene As...");

                ImGui.Separator();

                ImGui.MenuItem("Build Settings...");

                ImGui.Separator();

                ImGui.MenuItem("Exit");

                ImGui.EndMenu();
            }

            // ================= EDIT =================
            if (ImGui.BeginMenu("Edit"))
            {
                ImGui.MenuItem("Undo");
                ImGui.MenuItem("Redo");
                ImGui.Separator();

                ImGui.MenuItem("Cut");
                ImGui.MenuItem("Copy");
                ImGui.MenuItem("Paste");

                ImGui.Separator();

                if (ImGui.MenuItem("ImGui Demo"))
                {
                    m_ShowImGuiDemo = true;
                    ImGui.ShowDemoWindow(ref m_ShowImGuiDemo);
                }

                ImGui.MenuItem("Project Settings...");
                ImGui.MenuItem("Preferences...");

                ImGui.EndMenu();
            }

            // ================= WINDOW =================
            if (ImGui.BeginMenu("Window"))
            {
                // panel list placeholder
                ImGui.MenuItem("Panel A");
                ImGui.MenuItem("Panel B");
                ImGui.MenuItem("Panel C");

                ImGui.EndMenu();
            }

            // ================= HELP =================
            if (ImGui.BeginMenu("Help"))
            {
                ImGui.MenuItem("Engine Homepage");
                ImGui.MenuItem("GitHub");

                ImGui.EndMenu();
            }

            HandleWindowControl();

            ImGui.EndMainMenuBar();
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

        ImGui.PushStyleColor(ImGuiCol.Button, new System.Numerics.Vector4(0.1f, 0.1f, 0.1f, 0.1f));
        ImGui.PushStyleColor(ImGuiCol.ButtonHovered, new System.Numerics.Vector4(1, 1, 1, 0.2f));
        ImGui.PushStyleColor(ImGuiCol.ButtonActive, new System.Numerics.Vector4(1, 1, 1, 0.4f));
        ImGui.PushStyleColor(ImGuiCol.Border, new System.Numerics.Vector4(0, 0, 0, 0.01f));

        var size = new System.Numerics.Vector2(40, ImGui.GetFrameHeight());

        if (ImGui.Button(FontAwesome5Pro.Minus + "##MinEditor", size))
        {
            m_EditorWindow.Minimize();
        }

        if (m_EditorMaximized)
        {
            if (ImGui.Button(FontAwesome5Pro.WindowRestore + "##RestoreEditor", size))
            {
                //m_EditorWindow.Position = s_WindowSize;
                //m_EditorWindow.Size = s_WindowSize;
                //m_EditorWindow.SetResizable(true);
                m_EditorWindow.Restore();
                m_EditorMaximized = false;
            }
        }
        else
        {
            if (ImGui.Button(FontAwesome5Pro.WindowMaximize + "##MaximizeEditor", size))
            {
                m_EditorWindow.Maximize();
                m_EditorMaximized = true;
            }
        }
        if (ImGui.Button(FontAwesome5Pro.Times + "##close", size))
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
