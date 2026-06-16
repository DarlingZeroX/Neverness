namespace Neverness.Runtime.Assets.Streaming;

/// <summary>
/// 資產非同步加載請求。
/// </summary>
public sealed class LoadRequest
{
    /// <summary>資產 GUID。</summary>
    public GUID Guid;

    /// <summary>資產型別 ID。</summary>
    public ulong TypeId;

    /// <summary>加載優先級。</summary>
    public LoadPriority Priority;

    /// <summary>取消令牌。</summary>
    public CancellationToken CancellationToken;

    /// <summary>完成通知（TaskCompletionSource 回傳 Handle）。</summary>
    public TaskCompletionSource<ulong>? CompletionSource;

    /// <summary>相機距離（用於同優先級排序，距離越近越優先）。</summary>
    public float Distance;

    /// <summary>目標 mip level（紋理 streaming 用）。</summary>
    public int TargetMipLevel;
}

/// <summary>
/// 資產加載結果。
/// </summary>
public sealed class LoadResult
{
    /// <summary>資產 GUID。</summary>
    public GUID Guid;

    /// <summary>資產型別 ID。</summary>
    public ulong TypeId;

    /// <summary>加載的原始資料。</summary>
    public byte[]? Data;

    /// <summary>是否成功。</summary>
    public bool Success;

    /// <summary>錯誤訊息（Success=false 時）。</summary>
    public string? ErrorMessage;

    /// <summary>對應的完成源（用於通知調用方）。</summary>
    public TaskCompletionSource<ulong>? CompletionSource;

    /// <summary>原始請求的取消令牌。</summary>
    public CancellationToken CancellationToken;
}
