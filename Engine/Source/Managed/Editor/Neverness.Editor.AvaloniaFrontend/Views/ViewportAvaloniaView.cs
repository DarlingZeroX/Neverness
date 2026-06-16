using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Neverness.Editor.Core;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Core.Private;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.AvaloniaFrontend.Services;
using Neverness.Editor.AvaloniaFrontend.Viewport;
using Neverness.Editor.AvaloniaFrontend.Public;
using Neverness.Runtime.Engine;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 编辑器视口 Avalonia View——显示 3D 场景。
///
/// 实现细节：
/// - NativeControlHost 嵌入原生渲染窗口
/// - Diligent 通过 SwapChain 直接渲染到原生窗口
/// - Deferred Resize：BoundsProperty 变化 → MarkResize → 帧末 FlushResizes
/// - Surface Lost：HandleDestroyed → MarkSurfaceLost → HandleCreated → RecreateSurface
/// - 主线程渲染：AvaloniaFrontendModule.RegisterRenderCallback
///
/// 数据流：
///   Bind() → ViewportHostService.CreateSurface → new ViewportHostControl
///   HandleCreated → Registry.Register → surfaceId
///   Bounds 变化 → MarkResize(surfaceId, w, h)
///   TickRendering → FlushResizes → RenderViewport → Present
///   HandleDestroyed → MarkSurfaceLost
///   HandleCreated (重建) → RecreateSurface
/// </summary>
public class ViewportAvaloniaView : AvaloniaViewBase
{
    private EditorViewportViewModel? _viewModel;
    private EditorViewportController? _controller;
    private Panel? _viewportPanel;
    private TextBlock? _sizeLabel;

    // ── 视口表面管理 ──
    private ViewportHostService? _surfaceService;
    private IViewportSurfaceRegistry? _surfaceRegistry;
    private ulong _surfaceId;
    private bool _renderCallbackRegistered;

    // ── Deferred Resize ──
    private uint _pendingWidth;
    private uint _pendingHeight;
    private bool _resizePending;

    public ViewportAvaloniaView() : base("Viewport")
    {
    }

    public override Type ViewModelType => typeof(EditorViewportViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (EditorViewportViewModel)viewModel;

        // 获取 Surface Registry（由 SceneModule 注册）
        _surfaceRegistry = CoreModuleImp.Context.GetService<IViewportSurfaceRegistry>();

        // 创建 Avalonia 控件树
        var panel = new DockPanel();

        // ── 工具栏 ──
        var toolbar = CreateToolbar();
        DockPanel.SetDock(toolbar, Avalonia.Controls.Dock.Top);
        panel.Children.Add(toolbar);

        // ── 视口区域 ──
        _viewportPanel = new Panel
        {
            Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")),
            ClipToBounds = true,
        };

        // 创建视口表面
        _surfaceService = new ViewportHostService();
        var surface = _surfaceService.CreateSurface(
            (int)(_viewModel?.ViewportWidth ?? 800),
            (int)(_viewModel?.ViewportHeight ?? 600));

        // 监听原生句柄事件
        surface.SurfaceCreated += OnSurfaceCreated;
        surface.SurfaceDestroyed += OnSurfaceDestroyed;
        surface.SurfaceResized += OnSurfaceResized;

        // 将 ViewportHostControl 添加到面板
        var hostControl = _surfaceService.GetControl();
        if (hostControl != null)
        {
            _viewportPanel.Children.Add(hostControl);
        }

        // 监听 Bounds 变化（Deferred Resize）
        _viewportPanel.PropertyChanged += OnBoundsChanged;

        panel.Children.Add(_viewportPanel);

        Content = panel;

        // 订阅 ViewModel 变更
        _viewModel.PropertyChanged += OnPropertyChanged;

        // 注册主线程渲染回调
        RegisterRenderingCallback();
    }

    public override void Unbind()
    {
        // 注销渲染回调
        UnregisterRenderingCallback();

        if (_viewModel != null)
            _viewModel.PropertyChanged -= OnPropertyChanged;

        // 注销 Surface
        if (_surfaceId != 0 && _surfaceRegistry != null)
        {
            _surfaceRegistry.Unregister(_surfaceId);
            _surfaceId = 0;
        }

        _surfaceService?.Dispose();
        _surfaceService = null;
        _surfaceRegistry = null;
        _viewModel = null;
        _controller = null;
        _viewportPanel = null;
        _sizeLabel = null;
    }

    /// <summary>设置 Controller（由 AvaloniaViewFactory 调用）。</summary>
    public void SetController(EditorViewportController controller)
    {
        _controller = controller;
    }

    // ── 原生句柄事件 ──

    /// <summary>原生句柄创建完成——注册到 Surface Registry。</summary>
    private void OnSurfaceCreated(IntPtr handle)
    {
        if (_surfaceRegistry == null || _surfaceService?.Surface == null) return;

        var surface = _surfaceService.Surface;
        var handleDescriptor = (_surfaceService as ViewportHostService)?.GetHandleDescriptor() ?? "";

        // 根据 HandleDescriptor 确定 handleType
        uint handleType = handleDescriptor switch
        {
            "HWND" => (uint)NNNativeHandleType.Win32HWND,
            "X11" => (uint)NNNativeHandleType.X11Window,
            "NSView" => (uint)NNNativeHandleType.NSView,
            _ => 0
        };

        // 使用实际面板尺寸（ViewModel 可能还没初始化）
        var panelW = _viewportPanel?.Bounds.Width > 0 ? (uint)_viewportPanel.Bounds.Width : 800u;
        var panelH = _viewportPanel?.Bounds.Height > 0 ? (uint)_viewportPanel.Bounds.Height : 600u;
        var vmW = (uint)(_viewModel?.ViewportWidth ?? 0);
        var vmH = (uint)(_viewModel?.ViewportHeight ?? 0);

        var width = vmW > 0 ? vmW : panelW;
        var height = vmH > 0 ? vmH : panelH;

        // 如果是重建的句柄（Surface Lost 后），执行 RecreateSurface
        if (_surfaceId != 0)
        {
            var success = _surfaceRegistry.RecreateSurface(_surfaceId, handle, handleType);
            Console.WriteLine($"[ViewportAvaloniaView] RecreateSurface: surfaceId={_surfaceId}, success={success}");
            return;
        }

        // 首次注册
        _surfaceId = _surfaceRegistry.Register(handle, handleType, width, height);
        Console.WriteLine($"[ViewportAvaloniaView] Surface 注册成功: surfaceId={_surfaceId}, handle=0x{handle:X}, {width}x{height}");
    }

    /// <summary>原生句柄销毁——标记 Surface Lost。</summary>
    private void OnSurfaceDestroyed()
    {
        if (_surfaceId == 0 || _surfaceRegistry == null) return;

        _surfaceRegistry.MarkSurfaceLost(_surfaceId);
        Console.WriteLine($"[ViewportAvaloniaView] Surface Lost: surfaceId={_surfaceId}");
    }

    /// <summary>表面尺寸变更。</summary>
    private void OnSurfaceResized(int width, int height)
    {
        // 更新 ViewModel
        if (_viewModel != null)
        {
            _viewModel.ViewportWidth = width;
            _viewModel.ViewportHeight = height;
        }
    }

    // ── Deferred Resize ──

    /// <summary>Bounds 变化回调——标记 Deferred Resize。</summary>
    private void OnBoundsChanged(object? sender, AvaloniaPropertyChangedEventArgs e)
    {
        if (e.Property.Name != "Bounds") return;

        var bounds = _viewportPanel?.Bounds ?? default;
        if (bounds.Width <= 0 || bounds.Height <= 0) return;

        var w = (uint)bounds.Width;
        var h = (uint)bounds.Height;

        // 更新 ViewModel
        if (_viewModel != null)
        {
            _viewModel.ViewportWidth = (float)bounds.Width;
            _viewModel.ViewportHeight = (float)bounds.Height;
        }

        // 更新工具栏尺寸标签
        if (_sizeLabel != null)
            _sizeLabel.Text = $"{w}x{h}";

        // 标记 Deferred Resize（帧末统一执行）
        _pendingWidth = w;
        _pendingHeight = h;
        _resizePending = true;
    }

    // ── 主线程渲染 ──

    private void RegisterRenderingCallback()
    {
        if (_renderCallbackRegistered) return;
        AvaloniaFrontendModule.RegisterRenderCallback(OnMainThreadRender);
        _renderCallbackRegistered = true;
    }

    private void UnregisterRenderingCallback()
    {
        if (!_renderCallbackRegistered) return;
        AvaloniaFrontendModule.UnregisterRenderCallback(OnMainThreadRender);
        _renderCallbackRegistered = false;
    }

    /// <summary>主线程渲染回调——每帧由 TickRendering 调用。</summary>
    private void OnMainThreadRender()
    {
        if (_surfaceId == 0 || _surfaceRegistry == null) return;

        // 1. Deferred Resize：帧末统一执行
        if (_resizePending)
        {
            _resizePending = false;
            _surfaceRegistry.MarkResize(_surfaceId, _pendingWidth, _pendingHeight);
            _surfaceService?.Resize((int)_pendingWidth, (int)_pendingHeight);
        }

        // 2. FlushResizes（SwapChain ResizeBuffers）
        _surfaceRegistry.FlushResizes();

        // 3. 渲染视口（FBO → CopyTexture → SwapChain → Present）
        var sceneHandle = 0ul; // TODO: 从 Controller 获取当前场景句柄
        var w = (uint)(_viewModel?.ViewportWidth ?? 0);
        var h = (uint)(_viewModel?.ViewportHeight ?? 0);

        if (w > 0 && h > 0)
        {
            _surfaceRegistry.RenderViewport(_surfaceId, sceneHandle, w, h);
        }
    }

    // ── 工具栏 ──

    private StackPanel CreateToolbar()
    {
        var toolbar = new StackPanel
        {
            Orientation = Orientation.Horizontal,
            Spacing = 4,
            Margin = new Avalonia.Thickness(4),
        };

        // 聚焦按钮
        var focusButton = new Button
        {
            Content = "Focus",
            MinWidth = 60,
            MinHeight = 24,
            Padding = new Avalonia.Thickness(4, 2),
        };
        focusButton.Click += (_, _) =>
        {
            Console.WriteLine("[Viewport] Focus clicked");
        };
        toolbar.Children.Add(focusButton);

        // 网格切换
        var gridCheckBox = new CheckBox
        {
            Content = "Grid",
            IsChecked = true,
            VerticalAlignment = VerticalAlignment.Center,
        };
        gridCheckBox.IsCheckedChanged += (_, _) =>
        {
            Console.WriteLine($"[Viewport] Grid: {gridCheckBox.IsChecked}");
        };
        toolbar.Children.Add(gridCheckBox);

        // Gizmo 模式
        var gizmoCombo = new ComboBox
        {
            MinWidth = 100,
            MinHeight = 24,
            ItemsSource = new[] { "Translate", "Rotate", "Scale" },
            SelectedIndex = 0,
        };
        gizmoCombo.SelectionChanged += (_, _) =>
        {
            Console.WriteLine($"[Viewport] Gizmo mode: {gizmoCombo.SelectedIndex}");
        };
        toolbar.Children.Add(gizmoCombo);

        // 视口尺寸信息
        _sizeLabel = new TextBlock
        {
            Text = $"{_viewModel?.ViewportWidth ?? 0}x{_viewModel?.ViewportHeight ?? 0}",
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Avalonia.Thickness(8, 0),
            Foreground = new SolidColorBrush(Color.Parse("#FF999999")),
        };
        toolbar.Children.Add(_sizeLabel);

        return toolbar;
    }

    /// <summary>ViewModel 属性变更。</summary>
    private void OnPropertyChanged(string propertyName)
    {
        // TODO: 响应 ViewModel 属性变更
    }
}
