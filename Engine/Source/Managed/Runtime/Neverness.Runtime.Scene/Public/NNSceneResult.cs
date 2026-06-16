namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景操作结果枚举——兼容旧的 NNSceneResult 类型。
/// </summary>
public enum NNSceneResult : uint
{
    /// <summary>操作成功。</summary>
    Ok = 0,

    /// <summary>无效参数。</summary>
    Invalid = 1,

    /// <summary>未找到。</summary>
    NotFound = 2,

    /// <summary>已存在。</summary>
    AlreadyExists = 3,

    /// <summary>操作失败。</summary>
    Failed = 4,

    /// <summary>不支持的操作。</summary>
    NotSupported = 5,

    /// <summary>内存不足。</summary>
    OutOfMemory = 6,

    /// <summary>超时。</summary>
    Timeout = 7,

    /// <summary>被取消。</summary>
    Cancelled = 8,
}

/// <summary>
/// NNSceneResult 扩展方法。
/// </summary>
public static class NNSceneResultExtensions
{
    /// <summary>是否成功。</summary>
    public static bool IsSuccess(this NNSceneResult result) => result == NNSceneResult.Ok;

    /// <summary>是否失败。</summary>
    public static bool IsFailure(this NNSceneResult result) => result != NNSceneResult.Ok;

    /// <summary>从 bool 转换为 NNSceneResult。</summary>
    public static NNSceneResult ToNNSceneResult(this bool success) =>
        success ? NNSceneResult.Ok : NNSceneResult.Failed;

    /// <summary>从 NNSceneResult 转换为 bool。</summary>
    public static bool ToBool(this NNSceneResult result) => result == NNSceneResult.Ok;
}
