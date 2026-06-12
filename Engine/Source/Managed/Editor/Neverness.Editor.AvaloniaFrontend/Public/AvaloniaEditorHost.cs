using Neverness.Runtime.Application.Public;

namespace Neverness.Editor.AvaloniaFrontend.Public;

/// <summary>
/// Avalonia 编辑器宿主——管理 Avalonia 应用生命周期。
///
/// 职责：
/// - 启动 Avalonia 应用（独立线程）
/// - 与 Native 事件循环集成
/// - 管理模块安装顺序
/// - 协调 ImGui/Avalonia 共存
///
/// 使用方式：
///   var host = new AvaloniaEditorHost();
///   host.Start(nativeWindow);
///   // ... 主循环 ...
///   host.Shutdown();
/// </summary>
public class AvaloniaEditorHost : IDisposable
{
    private Thread? _avaloniaThread;
    private bool _isRunning;
    private bool _disposed;
    private readonly ManualResetEventSlim _startupEvent = new(false);

    /// <summary>Avalonia 应用是否已启动。</summary>
    public bool IsRunning => _isRunning;

    /// <summary>
    /// 启动 Avalonia 编辑器。
    /// </summary>
    /// <param name="nativeWindow">原生窗口（用于事件循环集成）。</param>
    public void Start(Window nativeWindow)
    {
        if (_isRunning)
        {
            Console.WriteLine("[AvaloniaEditorHost] 已在运行中");
            return;
        }

        Console.WriteLine("[AvaloniaEditorHost] 启动 Avalonia 编辑器...");

        // 在独立线程启动 Avalonia 应用
        _avaloniaThread = new Thread(() =>
        {
            try
            {
                _isRunning = true;
                _startupEvent.Set(); // 通知主线程 Avalonia 已启动
                Program.RunAvaloniaApp();
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"[AvaloniaEditorHost] Avalonia 应用异常: {ex}");
            }
            finally
            {
                _isRunning = false;
            }
        })
        {
            Name = "AvaloniaUIThread",
            IsBackground = true
        };
        _avaloniaThread.Start();

        // 等待 Avalonia 应用初始化完成
        _startupEvent.Wait(TimeSpan.FromSeconds(5));

        Console.WriteLine("[AvaloniaEditorHost] Avalonia 编辑器已启动");
    }

    /// <summary>
    /// 安装 AvaloniaFrontend 模块。
    /// 必须在 Framework.Install() 之后调用。
    /// </summary>
    public void InstallModule()
    {
        Console.WriteLine("[AvaloniaEditorHost] 安装 AvaloniaFrontend 模块...");
        AvaloniaFrontendModule.Install();
    }

    /// <summary>
    /// 安装主窗口。
    /// 必须在 Framework.Install() 之后、Core.Install() 之前调用。
    /// </summary>
    public void InstallShell(Window nativeWindow)
    {
        Console.WriteLine("[AvaloniaEditorHost] 安装主窗口...");
        AvaloniaFrontendModule.InstallShell(nativeWindow);
    }

    /// <summary>
    /// 注册上下文菜单贡献者。
    /// 必须在 EditorCompositionRoot.Build() 之后调用。
    /// </summary>
    public void RegisterContextMenuContributors()
    {
        AvaloniaFrontendModule.RegisterContextMenuContributors();
    }

    /// <summary>
    /// 关闭 Avalonia 编辑器。
    /// </summary>
    public void Shutdown()
    {
        if (!_isRunning)
            return;

        Console.WriteLine("[AvaloniaEditorHost] 关闭 Avalonia 编辑器...");

        // 关闭 Avalonia 应用
        Program.Shutdown();

        // 等待线程结束
        if (_avaloniaThread != null && _avaloniaThread.IsAlive)
        {
            _avaloniaThread.Join(5000); // 最多等待 5 秒
        }

        _isRunning = false;
        Console.WriteLine("[AvaloniaEditorHost] Avalonia 编辑器已关闭");
    }

    public void Dispose()
    {
        if (_disposed)
            return;

        _disposed = true;
        Shutdown();
        _startupEvent.Dispose();
    }
}
