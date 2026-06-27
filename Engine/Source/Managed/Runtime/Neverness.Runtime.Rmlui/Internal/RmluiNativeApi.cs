namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 文档热重载 API。
///
/// 委托给 RmlRendererManager → RmlDocumentManager，不再调用 C++ 函数指针。
///
/// 使用方法：
///   RmluiNativeApi.ReloadDocument("/assets/ui/main.html");
///   RmluiNativeApi.ReloadAllDocuments();
/// </summary>
public static class RmluiNativeApi
{
    /// <summary>
    /// 重载指定路径的文档（通知所有渲染器）。
    /// </summary>
    /// <param name="vfsPath">文档的 VFS 路径（如 "/assets/ui/main.html"）。</param>
    public static void ReloadDocument(string vfsPath)
    {
        RmlRendererManager.Instance.NotifyFileChanged(vfsPath);
        Console.WriteLine($"[RmluiNativeApi] 重载文档: {vfsPath}");
    }

    /// <summary>
    /// 重载所有渲染器的所有文档。
    /// </summary>
    public static void ReloadAllDocuments()
    {
        foreach (var surfaceId in RmlRendererManager.Instance.SurfaceIds)
        {
            var renderer = RmlRendererManager.Instance.GetRenderer(surfaceId);
            renderer?.DocumentManager.ReloadAll();
        }
        Console.WriteLine("[RmluiNativeApi] 全量重载所有文档");
    }

}
