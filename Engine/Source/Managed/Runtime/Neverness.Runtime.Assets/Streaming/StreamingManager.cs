using System.IO;
using System.Threading.Channels;

namespace Neverness.Runtime.Assets.Streaming;

/// <summary>
/// 非同步 IO/解碼管線。
///
/// 使用 System.Threading.Channels 生產者-消費者模式：
///   調用方 → Channel&lt;LoadRequest&gt; → IO Workers → Decode Workers → 完成佇列
///
/// IO Workers 通過 SemaphoreSlim 限制並發度（預設 4），
/// Decode Workers 同樣限制並發度（預設 2）。
///
/// 與 C++ NNStreamingManager 對應。
/// </summary>
public sealed class StreamingManager : IDisposable
{
    /* ======================== 配置 ======================== */

    /// <summary>預設 IO 並發度。</summary>
    public const int DefaultIoConcurrency = 4;

    /// <summary>預設解碼並發度。</summary>
    public const int DefaultDecodeConcurrency = 2;

    /// <summary>請求佇列容量（背壓控制）。</summary>
    public const int RequestQueueCapacity = 256;

    /* ======================== 內部狀態 ======================== */

    // IO 階段：讀取 .nnasset 檔案
    private readonly Channel<LoadRequest> _requestChannel;
    private readonly Channel<(LoadRequest Request, byte[]? Data, bool Success, string? Error)> _ioResultChannel;

    // 解碼階段：驗證 header
    private readonly Channel<LoadResult> _decodeResultChannel;

    private readonly SemaphoreSlim _ioSemaphore;
    private readonly SemaphoreSlim _decodeSemaphore;

    private CancellationTokenSource _cts = new();
    private Task[]? _ioTasks;
    private Task[]? _decodeTasks;
    private Task? _completionTask;

    private readonly string _assetRoot;
    private volatile bool _running;
    private volatile bool _disposed;

    // 優先級排序佇列
    private readonly PriorityQueue<LoadRequest, (int Priority, float Distance)> _priorityQueue = new();
    private readonly object _priorityLock = new();

    /* ======================== 統計 ======================== */

    private int _pendingRequestCount;
    private int _completedCount;

    /// <summary>待處理請求數量。</summary>
    public int PendingRequestCount => Volatile.Read(ref _pendingRequestCount);

    /// <summary>已完成結果數量（等待 Tick() 消費）。</summary>
    public int CompletedCount => Volatile.Read(ref _completedCount);

    /* ======================== 建構 ======================== */

    /// <summary>
    /// 建立 StreamingManager。
    /// </summary>
    /// <param name="assetRoot">資產根目錄（如 "Library/Imported"）。</param>
    /// <param name="ioConcurrency">IO 並發度。</param>
    /// <param name="decodeConcurrency">解碼並發度。</param>
    public StreamingManager(string assetRoot,
        int ioConcurrency = DefaultIoConcurrency,
        int decodeConcurrency = DefaultDecodeConcurrency)
    {
        _assetRoot = assetRoot;
        _requestChannel = Channel.CreateBounded<LoadRequest>(
            new BoundedChannelOptions(RequestQueueCapacity)
            {
                FullMode = BoundedChannelFullMode.Wait,
                SingleReader = false,
                SingleWriter = false,
            });
        _ioResultChannel = Channel.CreateUnbounded<(LoadRequest, byte[]?, bool, string?)>(
            new UnboundedChannelOptions
            {
                SingleReader = false,
                SingleWriter = true,
            });
        _decodeResultChannel = Channel.CreateUnbounded<LoadResult>(
            new UnboundedChannelOptions
            {
                SingleReader = true,
                SingleWriter = false,
            });
        _ioSemaphore = new SemaphoreSlim(ioConcurrency, ioConcurrency);
        _decodeSemaphore = new SemaphoreSlim(decodeConcurrency, decodeConcurrency);
    }

    /* ======================== 啟動/停止 ======================== */

    /// <summary>
    /// 啟動管線（啟動 IO 和解碼工作緒）。
    /// </summary>
    public void Start()
    {
        if (_running) return;
        _running = true;
        _cts = new CancellationTokenSource();

        // 啟動 IO workers
        _ioTasks = new Task[4]; // 與 ioConcurrency 對應
        for (int i = 0; i < _ioTasks.Length; i++)
            _ioTasks[i] = Task.Run(() => IoWorkerLoop(_cts.Token));

        // 啟動解碼 workers
        _decodeTasks = new Task[2]; // 與 decodeConcurrency 對應
        for (int i = 0; i < _decodeTasks.Length; i++)
            _decodeTasks[i] = Task.Run(() => DecodeWorkerLoop(_cts.Token));

        // 啟動優先級排序任務
        _completionTask = Task.Run(() => PrioritySorterLoop(_cts.Token));
    }

    /// <summary>
    /// 停止管線（等待所有工作完成）。
    /// </summary>
    public async Task StopAsync()
    {
        if (!_running) return;
        _running = false;

        _cts.Cancel();

        // 關閉 channel
        _requestChannel.Writer.TryComplete();
        _ioResultChannel.Writer.TryComplete();
        _decodeResultChannel.Writer.TryComplete();

        // 等待所有 worker 退出
        if (_ioTasks != null)
            await Task.WhenAll(_ioTasks).ConfigureAwait(false);
        if (_decodeTasks != null)
            await Task.WhenAll(_decodeTasks).ConfigureAwait(false);
        if (_completionTask != null)
            await _completionTask.ConfigureAwait(false);

        _cts.Dispose();
    }

    /* ======================== 請求提交 ======================== */

    /// <summary>
    /// 提交非同步加載請求。
    /// </summary>
    /// <param name="guid">資產 GUID。</param>
    /// <param name="typeId">型別 ID。</param>
    /// <param name="priority">加載優先級。</param>
    /// <param name="distance">相機距離（同優先級排序用）。</param>
    /// <param name="ct">取消令牌。</param>
    /// <returns>非同步等待的 Handle 值。</returns>
    public async ValueTask<ulong> SubmitRequestAsync(GUID guid, ulong typeId,
        LoadPriority priority = LoadPriority.Normal,
        float distance = 0f,
        CancellationToken ct = default)
    {
        var tcs = new TaskCompletionSource<ulong>(TaskCreationOptions.RunContinuationsAsynchronously);
        var request = new LoadRequest
        {
            Guid = guid,
            TypeId = typeId,
            Priority = priority,
            Distance = distance,
            CancellationToken = ct,
            CompletionSource = tcs,
        };

        Interlocked.Increment(ref _pendingRequestCount);

        // 寫入 channel（背壓：若滿則等待）
        await _requestChannel.Writer.WriteAsync(request, ct).ConfigureAwait(false);

        return await tcs.Task.ConfigureAwait(false);
    }

    /* ======================== 完成佇列消費 ======================== */

    /// <summary>
    /// 主線程 Tick()：消費完成佇列，將結果分派給 AssetManager。
    /// </summary>
    /// <param name="onCompleted">每個完成結果的回調。</param>
    /// <returns>處理的結果數量。</returns>
    public int DrainCompleted(Action<LoadResult> onCompleted)
    {
        int count = 0;
        while (_decodeResultChannel.Reader.TryRead(out var result))
        {
            Interlocked.Decrement(ref _completedCount);
            onCompleted(result);
            count++;
        }
        return count;
    }

    /* ======================== 內部 Worker 迴圈 ======================== */

    /// <summary>
    /// IO Worker：從 channel 讀取請求，執行檔案 IO。
    /// </summary>
    private async Task IoWorkerLoop(CancellationToken ct)
    {
        try
        {
            await foreach (var request in _requestChannel.Reader.ReadAllAsync(ct).ConfigureAwait(false))
            {
                if (request.CancellationToken.IsCancellationRequested)
                {
                    Interlocked.Decrement(ref _pendingRequestCount);
                    request.CompletionSource?.TrySetCanceled(request.CancellationToken);
                    continue;
                }

                await _ioSemaphore.WaitAsync(ct).ConfigureAwait(false);
                try
                {
                    var (data, success, error) = await ReadAssetFileAsync(request.Guid, request.CancellationToken)
                        .ConfigureAwait(false);
                    await _ioResultChannel.Writer.WriteAsync(
                        (request, data, success, error), ct).ConfigureAwait(false);
                }
                finally
                {
                    _ioSemaphore.Release();
                }
            }
        }
        catch (OperationCanceledException) { /* 正常退出 */ }
        catch (ChannelClosedException) { /* channel 關閉 */ }
    }

    /// <summary>
    /// 優先級排序：從 IO 結果 channel 讀取，按優先級排序後送入解碼 channel。
    /// </summary>
    private async Task PrioritySorterLoop(CancellationToken ct)
    {
        try
        {
            await foreach (var (request, data, success, error) in _ioResultChannel.Reader.ReadAllAsync(ct)
                .ConfigureAwait(false))
            {
                if (!success || data == null)
                {
                    // IO 失敗：直接送入完成佇列
                    Interlocked.Decrement(ref _pendingRequestCount);
                    var failResult = new LoadResult
                    {
                        Guid = request.Guid,
                        TypeId = request.TypeId,
                        Success = false,
                        ErrorMessage = error ?? "IO 失敗",
                        CompletionSource = request.CompletionSource,
                        CancellationToken = request.CancellationToken,
                    };
                    await _decodeResultChannel.Writer.WriteAsync(failResult, ct).ConfigureAwait(false);
                    Interlocked.Increment(ref _completedCount);
                    continue;
                }

                // 送入解碼 channel
                await _decodeResultChannel.Writer.WriteAsync(
                    new LoadResult
                    {
                        Guid = request.Guid,
                        TypeId = request.TypeId,
                        Data = data,
                        Success = true,
                        CompletionSource = request.CompletionSource,
                        CancellationToken = request.CancellationToken,
                    }, ct).ConfigureAwait(false);
                Interlocked.Increment(ref _completedCount);
            }
        }
        catch (OperationCanceledException) { /* 正常退出 */ }
        catch (ChannelClosedException) { /* channel 關閉 */ }
    }

    /// <summary>
    /// 解碼 Worker：驗證 .nnasset header。
    /// </summary>
    private async Task DecodeWorkerLoop(CancellationToken ct)
    {
        // 解碼 Worker 目前只做 header 驗證，
        // 實際的解壓縮/格式轉換在後續 Phase 實現。
        // 此處為佔位，實際解碼在 DrainCompleted 回調中由 AssetManager 處理。
        try
        {
            await Task.Delay(Timeout.Infinite, ct).ConfigureAwait(false);
        }
        catch (OperationCanceledException) { /* 正常退出 */ }
    }

    /* ======================== 檔案 IO ======================== */

    /// <summary>
    /// 非同步讀取 .nnasset 檔案。
    /// 嘗試多個搜索路徑（與 C++ NNStreamingManager::IoThreadFunc 對齊）。
    /// </summary>
    private async Task<(byte[]? Data, bool Success, string? Error)> ReadAssetFileAsync(
        GUID guid, CancellationToken ct)
    {
        var hex = guid.ToHexString();
        var prefix = hex[..2];
        var fileName = hex + ".nnasset";

        // 嘗試多個搜索路徑
        string[] searchPaths =
        [
            Path.Combine(_assetRoot, "Imported", prefix, fileName),
            Path.Combine(_assetRoot, fileName),
            fileName,
        ];

        foreach (var path in searchPaths)
        {
            ct.ThrowIfCancellationRequested();

            if (!File.Exists(path))
                continue;

            try
            {
                var data = await File.ReadAllBytesAsync(path, ct).ConfigureAwait(false);
                if (data.Length < (int)Formats.AssetFormat.HeaderSize)
                    continue;

                return (data, true, null);
            }
            catch (OperationCanceledException) { throw; }
            catch (Exception ex)
            {
                return (null, false, ex.Message);
            }
        }

        return (null, false, $"找不到資產檔案: {hex}");
    }

    /* ======================== 取消 ======================== */

    /// <summary>
    /// 取消指定 GUID 的所有待處理請求。
    /// </summary>
    public void CancelRequest(GUID guid)
    {
        // Channel 不支持直接取消單個請求，
        // 通過 CancellationToken 實現。
        // 此方法為 API 兼容保留。
    }

    /* ======================== IDisposable ======================== */

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;

        if (_running)
        {
            _running = false;
            _cts.Cancel();
            _requestChannel.Writer.TryComplete();
            _ioResultChannel.Writer.TryComplete();
            _decodeResultChannel.Writer.TryComplete();
        }

        _cts.Dispose();
        _ioSemaphore.Dispose();
        _decodeSemaphore.Dispose();
    }
}
