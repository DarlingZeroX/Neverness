using System.Runtime.InteropServices;

namespace Neverness.Runtime.Rmlui.Internal;

/// <summary>
/// C++ RmlUIRenderer ABI P/Invoke 声明。
///
/// 只暴露渲染器生命周期管理：
/// - Create: 创建 C++ RmlUIRenderer 实例，返回 Handle
/// - Destroy: 销毁实例
///
/// 其他逻辑（文档管理、输入处理、更新）全部在 C# 端通过 RmlUi.Net 实现。
/// C++ 端只负责 GPU 渲染，Handle 传给 ViewportSurface 执行渲染。
/// </summary>
internal static partial class RmlNativeInterop
{
    private const string NativeLib = "NevernessRuntime-RmlUI";

    /// <summary>
    /// 创建 C++ RmlUIRenderer 实例。
    /// </summary>
    /// <param name="width">视口宽度。</param>
    /// <param name="height">视口高度。</param>
    /// <returns>渲染器 Handle，0 表示失败。</returns>
    [LibraryImport(NativeLib, EntryPoint = "RmlRenderer_Create")]
    internal static partial uint RmlRenderer_Create(int width, int height);

    /// <summary>
    /// 销毁 C++ RmlUIRenderer 实例。
    /// </summary>
    /// <param name="handle">渲染器 Handle。</param>
    [LibraryImport(NativeLib, EntryPoint = "RmlRenderer_Destroy")]
    internal static partial void RmlRenderer_Destroy(uint handle);
}
