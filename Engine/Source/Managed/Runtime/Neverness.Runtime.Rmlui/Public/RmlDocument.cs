using RmlUiNet;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 文档封装。
///
/// 封装 RmlUi.Net 的 ElementDocument，提供文档生命周期管理。
/// </summary>
public sealed class RmlDocument : IDisposable
{
    /// <summary>所属渲染器。</summary>
    private readonly RmlRenderer _renderer;

    /// <summary>RmlUi.Net 文档实例。</summary>
    private ElementDocument? _document;

    /// <summary>文档路径。</summary>
    private readonly string _path;

    /// <summary>是否已释放。</summary>
    private bool _disposed;

    /// <summary>
    /// 创建文档实例。
    /// </summary>
    /// <param name="renderer">所属渲染器。</param>
    /// <param name="document">RmlUi.Net 文档实例。</param>
    /// <param name="path">文档路径。</param>
    internal RmlDocument(RmlRenderer renderer, ElementDocument document, string path)
    {
        _renderer = renderer;
        _document = document;
        _path = path;
    }

    /// <summary>
    /// 文档 VFS 路径。
    /// </summary>
    public string Path => _path;

    /// <summary>
    /// 文档是否有效。
    /// </summary>
    public bool IsValid => _document != null;

    #region 文档操作

    /// <summary>
    /// 显示文档。
    /// </summary>
    public void Show()
    {
        _document?.Show();
    }

    /// <summary>
    /// 隐藏文档。
    /// </summary>
    public void Hide()
    {
        _document?.Hide();
    }

    /// <summary>
    /// 重新加载文档。
    /// </summary>
    public void Reload()
    {
        if (_document == null) return;

        // 保存当前可见状态
        // TODO: 检查可见状态

        // 关闭旧文档
        _document.Close();
        _document = null;

        // 重新加载
        var context = _renderer.Context;
        if (context != null)
        {
            _document = context.LoadDocument(_path);
            if (_document != null)
            {
                Show();
            }
        }
    }

    /// <summary>
    /// 获取元素。
    /// </summary>
    /// <param name="id">元素 ID。</param>
    /// <returns>元素实例，未找到返回 null。</returns>
    public Element? GetElementById(string id)
    {
        return _document?.GetElementById(id);
    }

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
    /// 获取底层 ElementDocument 实例。
    /// </summary>
    public ElementDocument? GetDocument()
    {
        return _document;
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

    #region IDisposable

    public void Dispose()
    {
        if (_disposed) return;

        if (_document != null)
        {
            _document.Close();
            _document = null;
        }

        _renderer.RemoveDocument(this);
        _disposed = true;
    }

    #endregion
}
