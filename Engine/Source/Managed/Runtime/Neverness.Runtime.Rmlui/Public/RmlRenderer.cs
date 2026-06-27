using RmlUiNet;
using RmlUiNet.Input;
using Neverness.Runtime.Rmlui.Internal;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 渲染器封装。
///
/// 职责：
/// - 持有 C++ RmlUIRenderer 的 Handle（用于 ViewportSurface 渲染）
/// - 使用 RmlUi.Net 管理 Context、Document、输入、更新等逻辑
///
/// 架构：
/// - C++ 只管 GPU 渲染（RmlDiligentRenderInterface）
/// - C# 管理所有逻辑（文档、输入、更新）
/// - 渲染时把 Handle 传给 ViewportSurface
/// </summary>
public sealed class RmlRenderer : IDisposable
{
    /// <summary>C++ 渲染器 Handle。</summary>
    private uint _handle;

    /// <summary>RmlUi.Net Context，用于逻辑管理。</summary>
    private Context? _context;

    /// <summary>已加载的文档列表。</summary>
    private readonly List<RmlDocument> _documents = new();

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
    }

    /// <summary>
    /// 渲染器 Handle（传给 ViewportSurface 执行渲染）。
    /// </summary>
    public uint Handle => _handle;

    /// <summary>
    /// RmlUi.Net Context。
    /// </summary>
    internal Context? Context => _context;

    /// <summary>
    /// 调整视口尺寸。
    /// </summary>
    public void Resize(int width, int height)
    {
        _context?.SetDimensions(width, height);
    }

    #region 文档管理

    /// <summary>
    /// 加载文档。
    /// </summary>
    /// <param name="path">文档 VFS 路径。</param>
    /// <returns>文档实例，失败返回 null。</returns>
    public RmlDocument? LoadDocument(string path)
    {
        if (_context == null) return null;

        var doc = _context.LoadDocument(path);
        if (doc == null) return null;

        var rmlDoc = new RmlDocument(this, doc, path);
        _documents.Add(rmlDoc);
        return rmlDoc;
    }

    /// <summary>
    /// 卸载文档。
    /// </summary>
    internal void RemoveDocument(RmlDocument doc)
    {
        _documents.Remove(doc);
    }

    /// <summary>
    /// 重新加载所有文档。
    /// </summary>
    public void ReloadAllDocuments()
    {
        foreach (var doc in _documents)
            doc.Reload();
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
    /// 处理键盘按键。
    /// </summary>
    public bool OnKey(KeyIdentifier key, bool down, KeyModifier modifiers)
    {
        if (down)
            return _context?.ProcessKeyDown(key, modifiers) ?? false;
        else
            return _context?.ProcessKeyUp(key, modifiers) ?? false;
    }

    /// <summary>
    /// 处理文本输入。
    /// </summary>
    public bool OnTextInput(string text)
    {
        return _context?.ProcessTextInput(text) ?? false;
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
    /// <param name="path">字体文件路径。</param>
    /// <param name="fallback">是否作为 fallback 字体。</param>
    /// <returns>是否成功。</returns>
    public bool LoadFontFace(string path, bool fallback = false)
    {
        return Rml.LoadFontFace(path, fallback);
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
    /// 调试器是否可见。
    /// </summary>
    public bool IsDebuggerVisible => RmlUiNet.Debugger.IsVisible();

    #endregion

    #region IDisposable

    public void Dispose()
    {
        if (_disposed) return;

        // 释放所有文档
        foreach (var doc in _documents.ToArray())
            doc.Dispose();
        _documents.Clear();

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
