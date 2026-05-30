using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Private;
using Neverness.Editor.Framework.Public;
using System.Numerics;

namespace Neverness.Editor.Shell.Private;

/// <summary>
/// 编辑器主窗口——全屏 ImGui 窗口，承载 DockSpace 和所有子面板。
/// </summary>
public sealed class EditorMainWindow : IEditorPanel, IMainWindowHost
{
    // =========================================
    // Fields
    // =========================================

    private bool m_ItemDragging;

    private ImGuiWindowClass m_WindowClass;
    private ImGuiWindowClass m_DockSpaceWindowClass;

    private readonly List<IEditorPanel> m_Panels = new();

    private readonly Dictionary<string, IEditorPanel> m_IDPanels =
        new(StringComparer.Ordinal);

    private bool m_NeedInitializeWindowSize = false;

    private Vector2 m_InitializeWindowSize;

    private bool m_DockspaceLayoutInitialized = false;
    private bool m_DockspaceLayoutRequestRearrange = false;

    // =========================================
    // Constructor
    // =========================================

    public EditorMainWindow()
    {
        m_DockSpaceWindowClass = new ImGuiWindowClass
        {
            ClassId = ImGui.GetID("Scene Editor Window"),
            DockingAllowUnclassed = 1
        };
    }

    // =========================================
    // IEditorPanel
    // =========================================

    public void OnGUI()
    {
        var panelManager = PanelManager.Instance;

        //bool showDemo = true;
        //
        //ImGui.ShowDemoWindow(ref showDemo);
        //
        // 顶层 Window Class
        ImGui.SetNextWindowClass(ref panelManager.GetImGuiWindowClass());
        //ImGui.ShowDemoWindow();
        bool useWorkArea = true;

        ImGuiWindowFlags flags =
            ImGuiWindowFlags.NoScrollbar |
            ImGuiWindowFlags.NoDecoration |
            ImGuiWindowFlags.NoMove |
            ImGuiWindowFlags.NoSavedSettings |
            ImGuiWindowFlags.NoBringToFrontOnFocus;

        var viewport = ImGui.GetMainViewport();

        ImGui.SetNextWindowPos(
            useWorkArea ? viewport.WorkPos : viewport.Pos);

        ImGui.SetNextWindowSize(
            useWorkArea ? viewport.WorkSize : viewport.Size);

        if (ImGui.Begin("Main Editor", flags))
        {
            DrawToolbar();

            ImGui.DockSpace(
                m_DockSpaceWindowClass.ClassId,
                Vector2.Zero,
                ImGuiDockNodeFlags.None,
                ref m_DockSpaceWindowClass);

            DrawPanels();

            // 全局快捷键
            HandleKeyboardShortcuts();
        }

        ImGui.End();
    }

    public void OnUpdate(float delta)
    {
        foreach (var panel in m_Panels)
        {
            panel.OnUpdate(delta);
        }

        foreach (var panel in m_IDPanels.Values)
        {
            panel.OnUpdate(delta);
        }
    }

    public void OnFixedUpdate()
    {
    }

    public string GetWindowFullName()
    {
        return "Main Editor Window";
    }

    public string GetWindowName()
    {
        return "Main Editor Window";
    }

    public void OpenWindow(bool open)
    {
    }

    public bool IsWindowOpened()
    {
        return true;
    }

    public bool IsAsync()
    {
        return false;
    }

    // =========================================
    // Toolbar
    // =========================================

    private void DrawToolbar()
    {
        ImGui.PushStyleVar(
            ImGuiStyleVar.FramePadding,
            new Vector2(6, 8));

        ImGui.PushStyleColor(
            ImGuiCol.Button,
            new Vector4(0.1f, 0.1f, 0.1f, 0.1f));

        ImGui.PushStyleColor(
            ImGuiCol.ButtonHovered,
            new Vector4(1f, 1f, 1f, 0.2f));

        ImGui.PushStyleColor(
            ImGuiCol.ButtonActive,
            new Vector4(1f, 1f, 1f, 0.4f));

        ImGui.PushStyleColor(
            ImGuiCol.Border,
            new Vector4(0f, 0f, 0f, 0.01f));

        // =====================================
        // Save Scene
        // =====================================

        if (ImGui.Button("Save Scene"))
        {
            EditorMenuRegistry.ExecuteCommand("file.save");
        }

        ImGui.SameLine();

        // =====================================
        // Center Play Button
        // =====================================

        {
            float windowWidth = ImGui.GetWindowSize().X;
            float x = windowWidth * 0.5f;

            ImGui.SetCursorPosX(x);
        }

        // =====================================
        // Play / Stop / Pause（通过命令系统解耦，不直接引用 Scene 模块）
        // =====================================

        var playCmd = EditorMenuRegistry.FindCommand("scene.play");
        var isPlaying = playCmd?.IsChecked?.Invoke() ?? false;

        if (!isPlaying)
        {
            if (ImGui.Button(FontAwesome5Pro.Play + "##ScenePlay"))
            {
                EditorMenuRegistry.ExecuteCommand("scene.play");
            }
        }
        else
        {
            if (ImGui.Button(FontAwesome5Pro.Stop + "##SceneStop"))
            {
                EditorMenuRegistry.ExecuteCommand("scene.stop");
            }
            ImGui.SameLine();
            if (ImGui.Button(FontAwesome5Pro.Pause + "##ScenePause"))
            {
                EditorMenuRegistry.ExecuteCommand("scene.pause");
            }
        }

        ImGui.PopStyleColor(4);
        ImGui.PopStyleVar();
    }

    // =========================================
    // Keyboard Shortcuts
    // =========================================

    private static void HandleKeyboardShortcuts()
    {
        // Ctrl+S 保存场景
        if (ImGui.IsKeyDown(ImGuiKey.ModCtrl) && ImGui.IsKeyPressed(ImGuiKey.S))
        {
            EditorMenuRegistry.ExecuteCommand("file.save");
        }
    }

    // =========================================
    // Panels
    // =========================================

    private void DrawPanels()
    {
        foreach (var panel in m_Panels)
        {
            ImGui.SetNextWindowClass(ref m_DockSpaceWindowClass);

            SafeDrawPanel(panel);
        }

        foreach (var panel in m_IDPanels.Values)
        {
            ImGui.SetNextWindowClass(ref m_DockSpaceWindowClass);

            SafeDrawPanel(panel);
        }
    }

    private static void SafeDrawPanel(IEditorPanel panel)
    {
        try
        {
            panel.OnGUI();
        }
        catch (Exception ex)
        {
#if DEBUG
            Console.WriteLine(
                $"[EditorMainWindow] Panel Exception: {panel.GetType().Name}\n{ex}");
#endif
        }
    }

    // =========================================
    // IMainWindowHost
    // =========================================

    public void AddPanelWithID(string id, IEditorPanel panel)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(id);
        ArgumentNullException.ThrowIfNull(panel);

        m_IDPanels[id] = panel;
    }

    public IEditorPanel? GetPanelWithID(string id)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(id);

        return m_IDPanels.TryGetValue(id, out var panel)
            ? panel
            : null;
    }

    // =========================================
    // Panel Management（内部使用）
    // =========================================

    public void AddPanel(IEditorPanel panel)
    {
        ArgumentNullException.ThrowIfNull(panel);

        if (!m_Panels.Contains(panel))
        {
            m_Panels.Add(panel);
        }
    }

    public void TraversePanels(Action<IEditorPanel> callback)
    {
        ArgumentNullException.ThrowIfNull(callback);

        foreach (var panel in m_IDPanels.Values)
        {
            callback(panel);
        }
    }

    // =========================================
    // Dock Layout
    // =========================================

    public void RequestRearrangeLayout()
    {
        m_DockspaceLayoutRequestRearrange = true;
    }
}
