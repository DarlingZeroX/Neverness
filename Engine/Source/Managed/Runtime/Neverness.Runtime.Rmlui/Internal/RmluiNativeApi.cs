

using System.Runtime.InteropServices;
using System.Text;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI 原生 API 包装。
///
/// 提供 RmlUI 文档热重载的 C# 调用接口。
/// 底层通过 EngineNativeApiBootstrap.EngineApi.ViewportRender 调用 C++ 函数指针。
///
/// 使用方法：
///   RmluiNativeApi.ReloadDocument("/assets/ui/main.html");
///   RmluiNativeApi.ReloadAllDocuments();
///
/// @threadsafe 所有方法为无状态调用，线程安全。
/// C++ 端将请求入队，下一帧渲染前统一执行。
/// </summary>
public static unsafe class RmluiNativeApi
{
    /// <summary>通知 native 端重新加载指定文档。</summary>
    /// <param name="vfsPath">文档的 VFS 路径（如 "/assets/ui/main.html"）。</param>
    public static void ReloadDocument(string vfsPath)
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportRender.ReloadRmlDocument == null)
        {
            Console.WriteLine("[RmluiNativeApi] ReloadRmlDocument 未实现");
            return;
        }

        /* 将路径编码为 UTF-8 NUL 终结字节数组 */
        var pathBytes = Encoding.UTF8.GetBytes(vfsPath + '\0');
        fixed (byte* p = pathBytes)
        {
            api.ViewportRender.ReloadRmlDocument(p);
        }

        Console.WriteLine($"[RmluiNativeApi] 重载文档: {vfsPath}");
    }

    /// <summary>通知 native 端重新加载所有文档。</summary>
    public static void ReloadAllDocuments()
    {
        ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
        if (api.ViewportRender.ReloadAllRmlDocuments == null)
        {
            Console.WriteLine("[RmluiNativeApi] ReloadAllRmlDocuments 未实现");
            return;
        }

        api.ViewportRender.ReloadAllRmlDocuments();
        Console.WriteLine("[RmluiNativeApi] 全量重载所有文档");
    }

    /// <summary>RmlUI 原生 API 是否可用（函数指针已安装）。</summary>
    public static bool IsAvailable
    {
        get
        {
            ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
            return api.ViewportRender.ReloadRmlDocument != null
                && api.ViewportRender.ReloadAllRmlDocuments != null;
        }
    }
}
