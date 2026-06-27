using RmlUiNet;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 文档封装。
///
/// 封装 RmlUi.Net 的 ElementDocument，提供文档生命周期管理。
/// 独立管理，不依赖 RmlRenderer。
/// </summary>
public sealed class RmlDocument : IDisposable
{
    /// <summary>RmlUi.Net 文档实例。</summary>
    private ElementDocument? _document;

    /// <summary>文档路径。</summary>
    private readonly string _path;

    /// <summary>是否已释放。</summary>
    private bool _disposed;

    /// <summary>
    /// 创建文档实例。
    /// </summary>
    /// <param name="document">RmlUi.Net 文档实例。</param>
    /// <param name="path">文档路径。</param>
    public RmlDocument(ElementDocument document, string path)
    {
        _document = document;
        _path = path;
    }

    #region 属性

    /// <summary>
    /// 文档 VFS 路径。
    /// </summary>
    public string Path => _path;

    /// <summary>
    /// 文档是否有效。
    /// </summary>
    public bool IsValid => _document != null;

    /// <summary>
    /// 文档是否可见。
    /// </summary>
    public bool IsVisible => _document != null; // TODO: 实现真正的可见性检查

    #endregion

    #region 文档操作

    /// <summary>
    /// 显示文档。
    /// </summary>
    /// <param name="modalFlag">模态标志。</param>
    /// <param name="focusFlag">焦点标志。</param>
    public void Show(ModalFlag modalFlag = ModalFlag.None, FocusFlag focusFlag = FocusFlag.Auto)
    {
        _document?.Show(modalFlag, focusFlag);
    }

    /// <summary>
    /// 隐藏文档。
    /// </summary>
    public void Hide()
    {
        _document?.Hide();
    }

    /// <summary>
    /// 关闭文档。
    /// </summary>
    public void Close()
    {
        if (_document != null)
        {
            _document.Close();
            _document = null;
        }
    }

    /// <summary>
    /// 重新加载文档。
    /// </summary>
    public void Reload()
    {
        // TODO: 需要持有 Context 才能重新加载
        // 暂时只关闭
    }

    /// <summary>
    /// 将文档拉到最前面。
    /// </summary>
    public void PullToFront()
    {
        _document?.PullToFront();
    }

    #endregion

    #region 元素访问

    /// <summary>
    /// 通过 ID 获取元素。
    /// </summary>
    /// <param name="id">元素 ID。</param>
    /// <returns>元素实例，未找到返回 null。</returns>
    public Element? GetElementById(string id)
    {
        return _document?.GetElementById(id);
    }

    /// <summary>
    /// 通过 CSS 选择器获取元素。
    /// </summary>
    /// <param name="selector">CSS 选择器。</param>
    /// <returns>元素实例，未找到返回 null。</returns>
    public Element? QuerySelector(string selector)
    {
        return _document?.QuerySelector(selector);
    }

    #endregion

    #region 属性操作

    /// <summary>
    /// 获取标题。
    /// </summary>
    public string GetTitle()
    {
        return _document?.GetTitle() ?? string.Empty;
    }

    /// <summary>
    /// 设置标题。
    /// </summary>
    public void SetTitle(string title)
    {
        _document?.SetTitle(title);
    }

    /// <summary>
    /// 获取源 URL。
    /// </summary>
    public string GetSourceURL()
    {
        return _document?.GetSourceURL() ?? string.Empty;
    }

    #endregion

    #region 事件

    /// <summary>
    /// 添加事件监听器。
    /// </summary>
    public void AddEventListener(string eventName, EventListener listener, bool inCapturePhase = false)
    {
        _document?.AddEventListener(eventName, listener, inCapturePhase);
    }

    /// <summary>
    /// 移除事件监听器。
    /// </summary>
    public void RemoveEventListener(string eventName, EventListener listener, bool inCapturePhase = false)
    {
        _document?.RemoveEventListener(eventName, listener, inCapturePhase);
    }

    #endregion

    #region 底层访问

    /// <summary>
    /// 获取底层 ElementDocument 实例。
    /// </summary>
    public ElementDocument? GetDocument()
    {
        return _document;
    }

    /// <summary>
    /// 获取原生指针（用于高级操作）。
    /// </summary>
    public IntPtr NativePtr => _document?.NativePtr ?? IntPtr.Zero;

    #endregion

    #region IDisposable

    public void Dispose()
    {
        if (_disposed) return;

        if (_document != null)
        {
            _document.Close();
            _document = null;
        }

        _disposed = true;
    }

    #endregion
}
