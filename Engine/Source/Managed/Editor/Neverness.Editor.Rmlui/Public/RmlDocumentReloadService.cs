using Neverness.Editor.Assets;
using Neverness.Editor.Core.Public;
using Neverness.Runtime.Rmlui;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Rmlui;

/// <summary>
/// RmlUI 文档热重载服务。
///
/// 订阅 AssetReloaded 事件，当 .html/.css 文件变更时
/// 通知 native RmlUI 渲染器重新加载文档。
///
/// 使用方法：
///   var service = new RmlDocumentReloadService(eventBus);
///   // 自动订阅事件，无需手动调用
///
/// @threadsafe Reload 调用从主线程 Tick 触发（通过 HotReloadCoordinator 事件队列）。
/// C++ 端将请求入队，下一帧渲染前统一执行。
/// </summary>
public sealed class RmlDocumentReloadService
{
    private readonly IEditorEventBus _eventBus;

    /// <summary>创建热重载服务并订阅事件。</summary>
    public RmlDocumentReloadService(IEditorEventBus eventBus)
    {
        _eventBus = eventBus;
        _eventBus.Subscribe(EditorEventType.AssetReloaded, OnAssetReloaded);
    }

    private void OnAssetReloaded(EditorEvent evt)
    {
        if (evt.Payload is not AssetReloadedEventPayload payload)
            return;

        var ext = payload.Path.Extension.ToLowerInvariant();

        if (ext is ".html" or ".htm" or ".rml")
        {
            NVirtualPath vPath = (NVirtualPath)ProjectPaths.GetResourcePath(new NPath(payload.Path.FullPath));
            /* HTML 自身变更：重载指定文档 */
            RmluiNativeApi.ReloadDocument(vPath.FullPath);
        }
        else if (ext is ".css" or ".rcss")
        {
            /* CSS 变更：全量重载（Editor 场景文档数量少） */
            RmluiNativeApi.ReloadAllDocuments();
        }
    }

    /// <summary>取消订阅事件（Dispose 时调用）。</summary>
    public void Dispose()
    {
        _eventBus.Unsubscribe(EditorEventType.AssetReloaded, OnAssetReloaded);
    }
}
