using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.Private;
using Neverness.Editor.ImGuiEx;

namespace Neverness.Editor.ImGuiFrontend.Features;

/// <summary>
/// ImGui 窗口系统 Feature。注册窗口管理、通知、任务、停靠布局等服务。
///
/// 由 ModuleLifecycleManager 自动发现并按拓扑排序初始化。
/// 依赖 com.neverness.framework（确保 PanelManager 已就绪）。
///
/// 从 Core/Private/Features/ImGuiWindowFeature.cs 搬迁而来。
/// 原因：此 Feature 依赖 ImGuiEx（UI 相关），不应留在 Core 模块。
/// </summary>
public sealed class ImGuiWindowFeature : IEditorFeature
{
    private UIThreadDispatcher? m_Dispatcher;
    private ImWindowManager? m_WindowManager;
    private NotificationService? m_NotificationService;
    private ImDockLayoutService? m_DockLayoutService;
    private UITaskManager? m_TaskManager;

    /// <inheritdoc />
    public string FeatureId => "com.neverness.imgui-windows";

    /// <inheritdoc />
    public string DisplayName => "ImGui Window System";

    /// <inheritdoc />
    public IReadOnlyList<string> Dependencies => [];

    /// <inheritdoc />
    public void Initialize(IEditorContext context)
    {
        m_Dispatcher = new UIThreadDispatcher();
        m_WindowManager = new ImWindowManager();
        m_NotificationService = new NotificationService();
        m_DockLayoutService = new ImDockLayoutService();
        m_TaskManager = new UITaskManager(m_Dispatcher);

        // 注册到服务定位器，供其他 Feature 消费
        context.RegisterService<IImWindowManager>(m_WindowManager);
        context.RegisterService<INotificationService>(m_NotificationService);
        context.RegisterService<IImDockLayoutService>(m_DockLayoutService);
        context.RegisterService(m_Dispatcher);
        context.RegisterService(m_TaskManager);
    }

    /// <inheritdoc />
    public void Shutdown(IEditorContext context)
    {
        m_TaskManager?.CancelAll();
        m_WindowManager?.CloseAll();
        m_NotificationService?.DismissAll();

        m_TaskManager = null;
        m_DockLayoutService = null;
        m_NotificationService = null;
        m_WindowManager = null;
        m_Dispatcher = null;
    }

    /// <summary>逐帧调用：驱动窗口系统渲染。</summary>
    public static void Tick(float deltaTime)
    {
        var context = CoreModuleImp.Context;

        if (context.TryGetService<UIThreadDispatcher>(out var dispatcher))
        {
            dispatcher.ProcessQueue();
        }

        if (context.TryGetService<IImWindowManager>(out var windowManager))
        {
            windowManager.Update(deltaTime);
            windowManager.RenderAllWindows();
        }

        if (context.TryGetService<INotificationService>(out var notifications))
        {
            notifications.Update(deltaTime);
            notifications.Render();
        }
    }
}
