using RmlUiNet;
using RmlUiNet.Input;
using Neverness.Runtime.Rmlui.Internal;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 渲染器封装。
///
/// 职责：
/// - 持有 C++ RmlUIRenderer 的 Handle（用于 ViewportSurface 渲染）
/// - 管理 RmlUi.Net Context 生命周期
/// - 提供输入处理、更新、字体、调试等功能
///
/// 文档管理由 RmlDocumentManager 负责。
/// </summary>
public sealed class RmlRenderer : IDisposable
{
    /// <summary>C++ 渲染器 Handle。</summary>
    private uint _handle;

    /// <summary>RmlUi.Net Context，用于逻辑管理。</summary>
    private Context? _context;

    /// <summary>文档管理器。</summary>
    private RmlDocumentManager _documentManager;

    /// <summary>是否已释放。</summary>
    private bool _disposed;

    /// <summary>
    /// 创建渲染器。
    /// </summary>
    /// <param name="width">视口宽度。</param>
    /// <param name="height">视口高度。</param>
    public RmlRenderer(int width, int height)
    {
        // 创建 C++ 渲染器（用于 ViewportSurface 渲染）
        _handle = RmlNativeInterop.RmlRenderer_Create(width, height);
        if (_handle == 0)
            throw new InvalidOperationException("Failed to create RmlUI renderer");

        // 初始化 RmlUi.Net（用于逻辑管理）
        Rml.Initialise();

        // 创建 Context（使用公共 API）
        _context = Rml.CreateContext("Main", new Vector2i(width, height));
        if (_context == null)
            throw new InvalidOperationException("Failed to create RmlUI context");

        // 创建文档管理器
        _documentManager = new RmlDocumentManager(_context);
    }

    #region 属性

    /// <summary>
    /// 渲染器 Handle（传给 ViewportSurface 执行渲染）。
    /// </summary>
    public uint Handle => _handle;

    /// <summary>
    /// RmlUi.Net Context。
    /// </summary>
    public Context? Context => _context;

    /// <summary>
    /// 文档管理器。
    /// </summary>
    public RmlDocumentManager DocumentManager => _documentManager;

    /// <summary>
    /// 调试器是否可见。
    /// </summary>
    public bool IsDebuggerVisible => RmlUiNet.Debugger.IsVisible();

    #endregion

    #region 视口

    /// <summary>
    /// 调整视口尺寸。
    /// </summary>
    public void Resize(int width, int height)
    {
        _context?.SetDimensions(width, height);
    }

    #endregion

    #region 输入处理

    /// <summary>
    /// 处理鼠标移动。
    /// </summary>
    public bool OnMouseMove(int x, int y, KeyModifier modifiers)
    {
        return _context?.ProcessMouseMove(x, y, modifiers) ?? false;
    }

    /// <summary>
    /// 处理鼠标按键。
    /// </summary>
    public bool OnMouseButton(MouseButton button, bool down, KeyModifier modifiers)
    {
        if (down)
            return _context?.ProcessMouseButtonDown((int)button, modifiers) ?? false;
        else
            return _context?.ProcessMouseButtonUp((int)button, modifiers) ?? false;
    }

    /// <summary>
    /// 处理鼠标滚轮。
    /// </summary>
    public bool OnMouseWheel(float delta, KeyModifier modifiers)
    {
        return _context?.ProcessMouseWheel(new Vector2f(0, delta), modifiers) ?? false;
    }

    /// <summary>
    /// 处理键盘按键（带快捷键支持）。
    /// </summary>
    public bool OnKey(KeyIdentifier key, bool down, KeyModifier modifiers, float nativeDpRatio = 1.0f)
    {
        if (!down) return _context?.ProcessKeyUp(key, modifiers) ?? false;

        // 优先级快捷键（在 Context 处理前）
        bool propagate = RmlShell.ProcessKeyDownShortcuts(
            _context!, key, modifiers, nativeDpRatio, priority: true);

        if (!propagate) return true;

        // 提交给 Context 处理
        bool handled = _context?.ProcessKeyDown(key, modifiers) ?? false;

        if (!handled)
        {
            // 低优先级快捷键（在 Context 未处理时）
            propagate = RmlShell.ProcessKeyDownShortcuts(
                _context!, key, modifiers, nativeDpRatio, priority: false);
        }

        return handled || !propagate;
    }

    /// <summary>
    /// 处理文本输入。
    /// </summary>
    public bool OnTextInput(string text)
    {
        return _context?.ProcessTextInput(text) ?? false;
    }

    /// <summary>
    /// 处理鼠标离开窗口。
    /// </summary>
    public void OnMouseLeave()
    {
        _context?.ProcessMouseLeave();
    }

    #endregion

    #region 更新

    /// <summary>
    /// 每帧更新。
    /// </summary>
    public void Update(float deltaTime)
    {
        _context?.Update();
    }

    #endregion

    #region 字体管理

    /// <summary>
    /// 加载字体。
    /// </summary>
    public bool LoadFontFace(string path, bool fallback = false)
    {
        return Rml.LoadFontFace(path, fallback);
    }

    /// <summary>
    /// 加载默认字体集。
    /// </summary>
    public void LoadDefaultFonts(string fontsPath)
    {
        RmlShell.LoadFonts(fontsPath);
    }

    #endregion

    #region 调试

    /// <summary>
    /// 设置调试器可见性。
    /// </summary>
    public void SetDebuggerVisible(bool visible)
    {
        RmlUiNet.Debugger.SetVisible(visible);
    }

    /// <summary>
    /// 切换调试器可见性。
    /// </summary>
    public void ToggleDebugger()
    {
        SetDebuggerVisible(!IsDebuggerVisible);
    }

    #endregion

    #region IDisposable

    public void Dispose()
    {
        if (_disposed) return;

        // 释放文档管理器
        _documentManager.Clear();

        // 释放 Context
        _context?.Dispose();
        _context = null;

        // 销毁 C++ 渲染器
        if (_handle != 0)
        {
            RmlNativeInterop.RmlRenderer_Destroy(_handle);
            _handle = 0;
        }

        // 关闭 RmlUi
        Rml.Shutdown();

        _disposed = true;
    }

    #endregion
}
