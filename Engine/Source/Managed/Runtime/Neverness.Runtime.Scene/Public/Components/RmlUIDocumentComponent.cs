using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene.Components;

/// <summary>
/// RmlUI 文档视图目标——控制在哪个视图中显示（与 Native NNRmlUIViewTarget 对齐）。
/// </summary>
public enum RmlUIViewTarget : uint
{
    /// <summary>仅在 Scene View 中显示（编辑模式预览）。</summary>
    Scene = 0,
    /// <summary>仅在 Game View 中显示（播放模式）。</summary>
    Game = 1,
    /// <summary>两个视图都显示。</summary>
    Both = 2,
}

/// <summary>
/// RmlUI 文档组件标志位（与 Native NNRmlUIDocumentFlags 对齐）。
/// </summary>
[Flags]
public enum RmlUIDocumentFlags : uint
{
    None = 0,
    /// <summary>场景启动时自动加载文档。</summary>
    AutoLoad = 1u << 0,
    /// <summary>可接收焦点。</summary>
    Focusable = 1u << 1,
    /// <summary>可接收输入事件。</summary>
    ReceivesInput = 1u << 2,
}

/// <summary>
/// RmlUI 文档组件——嵌入式 UI 文档。
/// 替代 C++ NNRmlUIDocumentComponent（32 字节）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct RmlUIDocumentComponent : IComponent
{
    /// <summary>HTML 文档资产 GUID。</summary>
    public NNGuid DocumentAsset;

    /// <summary>标志位。</summary>
    public RmlUIDocumentFlags Flags;

    /// <summary>排序顺序。</summary>
    public int SortOrder;

    /// <summary>视图目标。</summary>
    public RmlUIViewTarget ViewTarget;

    private uint _padding0;

    /// <summary>创建默认 RmlUI 文档组件。</summary>
    public static RmlUIDocumentComponent Default => new()
    {
        ViewTarget = RmlUIViewTarget.Scene,
    };
}
