using Hexa.NET.ImGui;
using System;
using System.Collections.Generic;
using Neverness.Editor.Framework.Interface;

namespace Neverness.Editor.Framework.Private
{
    /// <summary>
    /// 全局 Panel 管理器
    /// </summary>
    public sealed class PanelManager : IPanel, IPanelManager
    {
        // =========================
        // Singleton
        // =========================

        private static readonly Lazy<PanelManager> s_Instance =
            new(() => new PanelManager());

        public static PanelManager Instance => s_Instance.Value;

        // =========================
        // Members
        // =========================

        private readonly List<IEditorPanel> m_Panels = new();

        private readonly Dictionary<string, IEditorPanel> m_IdPanels =
            new(StringComparer.Ordinal);

        private ImGuiWindowClass m_TopLevelClass;

        private readonly uint m_TopLevelDockId;

        /// <summary>主窗口宿主回调——Shell 模块 Install 时注册。</summary>
        private Func<string, IEditorPanel, bool>? m_AddChildPanelCallback;

        // =========================
        // Constructor
        // =========================

        private PanelManager()
        {
            m_TopLevelDockId = ImGui.GetID("TopLevelDockSpace");

            m_TopLevelClass = new ImGuiWindowClass
            {
                ClassId = m_TopLevelDockId,

                // 允许普通窗口停靠
                DockingAllowUnclassed = 1
            };
        }

        // =========================
        // Panel Register
        // =========================

        /// <summary>
        /// 添加普通面板
        /// </summary>
        public bool AddPanel(IEditorPanel panel)
        {
            ArgumentNullException.ThrowIfNull(panel);

            if (m_Panels.Contains(panel))
            {
                return false;
            }

            m_Panels.Add(panel);
            return true;
        }

        /// <summary>
        /// 添加带 ID 的面板
        /// </summary>
        public bool AddPanelWithID(string id, IEditorPanel panel)
        {
            ArgumentException.ThrowIfNullOrWhiteSpace(id);
            ArgumentNullException.ThrowIfNull(panel);

            if (m_IdPanels.ContainsKey(id))
            {
                return false;
            }

            m_IdPanels.Add(id, panel);
            return true;
        }

        /// <summary>
        /// 移除普通面板
        /// </summary>
        public bool RemovePanel(IEditorPanel panel)
        {
            ArgumentNullException.ThrowIfNull(panel);

            return m_Panels.Remove(panel);
        }

        /// <summary>
        /// 移除带 ID 的面板
        /// </summary>
        public bool RemovePanel(string id)
        {
            ArgumentException.ThrowIfNullOrWhiteSpace(id);

            return m_IdPanels.Remove(id);
        }

        /// <summary>
        /// 清空全部面板
        /// </summary>
        public void Clear()
        {
            m_Panels.Clear();
            m_IdPanels.Clear();
        }

        // =========================
        // Query
        // =========================

        public IEditorPanel? GetPanelWithID(string id)
        {
            ArgumentException.ThrowIfNullOrWhiteSpace(id);

            return m_IdPanels.TryGetValue(id, out var panel)
                ? panel
                : null;
        }

        public bool HasPanel(string id)
        {
            ArgumentException.ThrowIfNullOrWhiteSpace(id);

            return m_IdPanels.ContainsKey(id);
        }

        // =========================
        // Window Control
        // =========================

        public bool OpenPanel(string id)
        {
            var panel = GetPanelWithID(id);
            if (panel == null)
            {
                return false;
            }

            panel.OpenWindow(true);
            return true;
        }

        public bool ClosePanel(string id)
        {
            var panel = GetPanelWithID(id);
            if (panel == null)
            {
                return false;
            }

            panel.OpenWindow(false);
            return true;
        }

        // =========================
        // IPanel
        // =========================

        /// <summary>
        /// PanelManager 本身不是异步 UI
        /// </summary>
        public bool IsAsync()
        {
            return false;
        }

        public void OnGUI()
        {
            // 如果启用了 Docking：
            //
            // unsafe
            // {
            //     ImGui.DockSpaceOverViewport(
            //         ImGui.GetMainViewport(),
            //         ImGuiDockNodeFlags.None,
            //         &m_TopLevelClass);
            // }

            DrawPanels(isAsync: false);
        }

        public void TickAsyncUI()
        {
            DrawPanels(isAsync: true);
        }

        public void OnUpdate(float delta)
        {
            foreach (var panel in m_Panels)
            {
                panel.OnUpdate(delta);
            }

            foreach (var panel in m_IdPanels.Values)
            {
                panel.OnUpdate(delta);
            }
        }

        public void OnFixedUpdate()
        {
            foreach (var panel in m_Panels)
            {
                panel.OnFixedUpdate();
            }

            foreach (var panel in m_IdPanels.Values)
            {
                panel.OnFixedUpdate();
            }
        }

        // =========================
        // Internal
        // =========================

        private void DrawPanels(bool isAsync)
        {
            foreach (var panel in m_Panels)
            {
                if (panel.IsAsync() == isAsync)
                {
                    SafeDrawPanel(panel);
                }
            }

            foreach (var panel in m_IdPanels.Values)
            {
                if (panel.IsAsync() == isAsync)
                {
                    SafeDrawPanel(panel);
                }
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
                    $"[PanelManager] Panel GUI Exception: {panel.GetType().Name}\n{ex}");
#endif
            }
        }

        // =========================
        // ImGui
        // =========================

        public ref ImGuiWindowClass GetImGuiWindowClass()
        {
            return ref m_TopLevelClass;
        }

        public uint GetWindowDockID()
        {
            return m_TopLevelDockId;
        }

        // =========================
        // IPanelManager.AddChildPanel
        // =========================

        /// <summary>注册主窗口宿主回调（Shell 模块 Install 时调用）。</summary>
        public void RegisterMainWindowCallback(Func<string, IEditorPanel, bool> callback)
        {
            ArgumentNullException.ThrowIfNull(callback);
            m_AddChildPanelCallback = callback;
        }

        /// <summary>向主窗口添加子面板（通过回调委托，不依赖具体类型）。</summary>
        public bool AddChildPanel(string id, IEditorPanel panel)
        {
            if (m_AddChildPanelCallback != null)
            {
                return m_AddChildPanelCallback(id, panel);
            }
            return false;
        }
    }
}
