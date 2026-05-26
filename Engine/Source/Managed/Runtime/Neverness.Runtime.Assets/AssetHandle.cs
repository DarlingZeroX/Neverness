using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Assets;

/// <summary>
/// 類型化資產 Handle，blittable，可跨 P/Invoke 傳遞。
/// 與 Native NNAssetHandleT&lt;void&gt; / NNAssetHandle（uint64）對應。
/// </summary>
/// <typeparam name="T">資產型別（僅用於型別安全，不影響記憶體佈局）。</typeparam>
[StructLayout(LayoutKind.Sequential)]
public readonly struct AssetHandle<T> : IEquatable<AssetHandle<T>>
{
    /// <summary>原始值（HandleTable 索引 + generation 編碼）。</summary>
    public ulong Value { get; }

    /// <summary>是否為零（無效 Handle）。</summary>
    public bool IsZero => Value == 0;

    /// <summary>全零無效 Handle。</summary>
    public static AssetHandle<T> Zero => default;

    public AssetHandle(ulong value)
    {
        Value = value;
    }

    /* ========== 查詢 ========== */

    /// <summary>資產是否已載入。</summary>
    public unsafe bool IsLoaded
    {
        get
        {
            if (IsZero) return false;
            var api = NativeApiProvider.AssetManagerApi;
            return api.IsAssetLoaded != null && api.IsAssetLoaded(Value) != 0;
        }
    }

    /// <summary>資產是否正在載入中。</summary>
    public unsafe bool IsLoading
    {
        get
        {
            if (IsZero) return false;
            var api = NativeApiProvider.AssetManagerApi;
            return api.IsAssetLoading != null && api.IsAssetLoading(Value) != 0;
        }
    }

    /// <summary>取得資產之 GUID。</summary>
    public unsafe GUID GetGuid()
    {
        if (IsZero) return GUID.Zero;
        var api = NativeApiProvider.AssetManagerApi;
        if (api.GetGuidByAsset == null) return GUID.Zero;
        var native = api.GetGuidByAsset(Value);
        return GUID.FromNative(native);
    }

    /* ========== 引用計數 ========== */

    /// <summary>增加引用計數。</summary>
    public unsafe void AddRef()
    {
        if (IsZero) return;
        if (NativeApiProvider.AssetManagerApi.AddRef != null) NativeApiProvider.AssetManagerApi.AddRef(Value);
    }

    /// <summary>減少引用計數。</summary>
    public unsafe void Release()
    {
        if (IsZero) return;
        if (NativeApiProvider.AssetManagerApi.ReleaseRef != null) NativeApiProvider.AssetManagerApi.ReleaseRef(Value);
    }

    /// <summary>取得當前引用計數。</summary>
    public unsafe uint GetRefCount()
    {
        if (IsZero) return 0;
        var api = NativeApiProvider.AssetManagerApi;
        return api.GetRefCount != null ? api.GetRefCount(Value) : 0u;
    }

    /* ========== 資料存取 ========== */

    /// <summary>取得資產原始資料大小（位元組）。</summary>
    public unsafe ulong GetDataSize()
    {
        if (IsZero) return 0;
        var api = NativeApiProvider.AssetManagerApi;
        return api.GetAssetDataSize != null ? api.GetAssetDataSize(Value) : 0ul;
    }

    /// <summary>取得資產中 blob 數量。</summary>
    public unsafe uint GetBlobCount()
    {
        if (IsZero) return 0;
        var api = NativeApiProvider.AssetManagerApi;
        return api.GetBlobCount != null ? api.GetBlobCount(Value) : 0u;
    }

    /// <summary>取得指定 blob 的大小。</summary>
    public unsafe ulong GetBlobSize(uint index)
    {
        if (IsZero) return 0;
        var api = NativeApiProvider.AssetManagerApi;
        return api.GetBlobSize != null ? api.GetBlobSize(Value, index) : 0ul;
    }

    /* ========== 運算子 ========== */

    public bool Equals(AssetHandle<T> other) => Value == other.Value;
    public override bool Equals(object? obj) => obj is AssetHandle<T> h && Equals(h);
    public override int GetHashCode() => Value.GetHashCode();
    public override string ToString() => IsZero ? "AssetHandle(Zero)" : $"AssetHandle(0x{Value:x16})";

    public static bool operator ==(AssetHandle<T> left, AssetHandle<T> right) => left.Equals(right);
    public static bool operator !=(AssetHandle<T> left, AssetHandle<T> right) => !left.Equals(right);

    /// <summary>從原始值建構。</summary>
    public static AssetHandle<T> FromRaw(ulong value) => new(value);
}

/// <summary>
/// 非型別化資產 Handle（簡化用）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public readonly unsafe struct AssetHandle : IEquatable<AssetHandle>
{
    public ulong Value { get; }
    public bool IsZero => Value == 0;
    public static AssetHandle Zero => default;

    public AssetHandle(ulong value) => Value = value;

    public bool IsLoaded
    {
        get
        {
            if (IsZero) return false;
            var api = NativeApiProvider.AssetManagerApi;
            return api.IsAssetLoaded != null && api.IsAssetLoaded(Value) != 0;
        }
    }

    public unsafe void AddRef()
    {
        if (IsZero) return;
        if (NativeApiProvider.AssetManagerApi.AddRef != null) NativeApiProvider.AssetManagerApi.AddRef(Value);
    }

    public unsafe void Release()
    {
        if (IsZero) return;
        if (NativeApiProvider.AssetManagerApi.ReleaseRef != null) NativeApiProvider.AssetManagerApi.ReleaseRef(Value);
    }

    public bool Equals(AssetHandle other) => Value == other.Value;
    public override bool Equals(object? obj) => obj is AssetHandle h && Equals(h);
    public override int GetHashCode() => Value.GetHashCode();
    public override string ToString() => IsZero ? "AssetHandle(Zero)" : $"AssetHandle(0x{Value:x16})";

    public static bool operator ==(AssetHandle left, AssetHandle right) => left.Equals(right);
    public static bool operator !=(AssetHandle left, AssetHandle right) => !left.Equals(right);
}

/* ========== 原生 API 存取 ========== */

/// <summary>
/// Native API 存取器。
/// </summary>
public static unsafe class NativeApiProvider
{
    /// <summary>AssetManagerAPI 函數表。</summary>
    public static AssetManagerApiTable AssetManagerApi { get; set; } = new();

    /// <summary>取得 AssetManager API（便捷方法）。</summary>
    public static AssetManagerApiTable Manager => AssetManagerApi;

    /// <summary>
    /// 從 <see cref="EngineNativeApiCache"/> 橋接函數指針到 <see cref="AssetManagerApi"/>。
    /// 必須在 Editor 啟動時、首次資產操作前調用。
    /// </summary>
    public static void WireFromEngineCache()
    {
        if (!EngineNativeApiCache.IsInstalled)
        {
            Console.WriteLine("[NativeApiProvider] EngineNativeApiCache 未安裝，跳過接線");
            return;
        }

        ref readonly var native = ref EngineNativeApiCache.EngineApi.AssetManager;

        AssetManagerApi = new AssetManagerApiTable
        {
            LoadAssetSync = (delegate* unmanaged[Stdcall]<NNGuid, ulong, ulong>)native.LoadAssetSync,
            LoadAssetAsync = (delegate* unmanaged[Stdcall]<NNGuid, ulong, int, void*, void*, ulong>)native.LoadAssetAsync,
            UnloadAsset = (delegate* unmanaged[Stdcall]<ulong, void>)native.UnloadAsset,
            UnloadAssetByGuid = (delegate* unmanaged[Stdcall]<NNGuid, void>)native.UnloadAssetByGuid,
            IsAssetLoaded = (delegate* unmanaged[Stdcall]<ulong, int>)native.IsAssetLoaded,
            IsAssetLoading = (delegate* unmanaged[Stdcall]<ulong, int>)native.IsAssetLoading,
            GetAssetByGuid = (delegate* unmanaged[Stdcall]<NNGuid, ulong>)native.GetAssetByGuid,
            GetGuidByAsset = (delegate* unmanaged[Stdcall]<ulong, NNGuid>)native.GetGuidByAsset,
            AddRef = (delegate* unmanaged[Stdcall]<ulong, void>)native.AddRef,
            ReleaseRef = (delegate* unmanaged[Stdcall]<ulong, void>)native.ReleaseRef,
            GetRefCount = (delegate* unmanaged[Stdcall]<ulong, uint>)native.GetRefCount,
            GetAssetData = (delegate* unmanaged[Stdcall]<ulong, void*>)native.GetAssetData,
            GetAssetDataSize = (delegate* unmanaged[Stdcall]<ulong, ulong>)native.GetAssetDataSize,
            GetBlobCount = (delegate* unmanaged[Stdcall]<ulong, uint>)native.GetBlobCount,
            GetBlobData = (delegate* unmanaged[Stdcall]<ulong, uint, void*>)native.GetBlobData,
            GetBlobSize = (delegate* unmanaged[Stdcall]<ulong, uint, ulong>)native.GetBlobSize,
            MountPackage = (delegate* unmanaged[Stdcall]<byte*, int>)native.MountPackage,
            UnmountPackage = (delegate* unmanaged[Stdcall]<byte*, void>)native.UnmountPackage,
            IsAssetInPackage = (delegate* unmanaged[Stdcall]<NNGuid, int>)native.IsAssetInPackage,
            MarkForReload = (delegate* unmanaged[Stdcall]<NNGuid, void>)native.MarkForReload,
            ReloadMarkedAssets = (delegate* unmanaged[Stdcall]<void>)native.ReloadMarkedAssets,
            GetLoadedAssetCount = (delegate* unmanaged[Stdcall]<ulong>)native.GetLoadedAssetCount,
            GetTotalMemoryUsage = (delegate* unmanaged[Stdcall]<ulong>)native.GetTotalMemoryUsage,
            InitializeAssetManager = native.initializeAssetManager,
        };

        Console.WriteLine("[NativeApiProvider] AssetManager API 已接線");
    }

    /// <summary>
    /// 初始化 Native AssetManager（設定項目根路徑）。
    /// </summary>
    /// <param name="projectRoot">項目根目錄絕對路徑（Library 的父目錄）。</param>
    /// <returns>是否成功。</returns>
    public static bool InitializeAssetManager(string projectRoot)
    {
        var api = AssetManagerApi;
        if (api.InitializeAssetManager == null)
        {
            Console.WriteLine("[NativeApiProvider] InitializeAssetManager 函數指針為 null");
            return false;
        }

        var utf8 = System.Text.Encoding.UTF8.GetBytes(projectRoot);
        fixed (byte* p = utf8)
        {
            var result = api.InitializeAssetManager(p);
            if (result != 0)
                Console.WriteLine($"[NativeApiProvider] AssetManager 初始化成功: {projectRoot}");
            else
                Console.WriteLine($"[NativeApiProvider] AssetManager 初始化失敗: {projectRoot}");
            return result != 0;
        }
    }
}

/// <summary>
/// 資產管理器 C# API 表。
/// 對應 Native NNAssetManagerAPI 結構（22 個函數）。
/// </summary>
public sealed unsafe class AssetManagerApiTable
{
    /* ---------- 同步載入 ---------- */
    public delegate* unmanaged[Stdcall]<NNGuid, ulong, ulong> LoadAssetSync { get; init; }

    /* ---------- 異步載入 ---------- */
    public delegate* unmanaged[Stdcall]<NNGuid, ulong, int, void*, void*, ulong> LoadAssetAsync { get; init; }

    /* ---------- 卸載 ---------- */
    public delegate* unmanaged[Stdcall]<ulong, void> UnloadAsset { get; init; }
    public delegate* unmanaged[Stdcall]<NNGuid, void> UnloadAssetByGuid { get; init; }

    /* ---------- 查詢 ---------- */
    public delegate* unmanaged[Stdcall]<ulong, int> IsAssetLoaded { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, int> IsAssetLoading { get; init; }
    public delegate* unmanaged[Stdcall]<NNGuid, ulong> GetAssetByGuid { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, NNGuid> GetGuidByAsset { get; init; }

    /* ---------- 引用計數 ---------- */
    public delegate* unmanaged[Stdcall]<ulong, void> AddRef { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, void> ReleaseRef { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, uint> GetRefCount { get; init; }

    /* ---------- 資料存取 ---------- */
    public delegate* unmanaged[Stdcall]<ulong, void*> GetAssetData { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, ulong> GetAssetDataSize { get; init; }

    /* ---------- Blob 存取 ---------- */
    public delegate* unmanaged[Stdcall]<ulong, uint> GetBlobCount { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, uint, void*> GetBlobData { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, uint, ulong> GetBlobSize { get; init; }

    /* ---------- 包管理 ---------- */
    public delegate* unmanaged[Stdcall]<byte*, int> MountPackage { get; init; }
    public delegate* unmanaged[Stdcall]<byte*, void> UnmountPackage { get; init; }
    public delegate* unmanaged[Stdcall]<NNGuid, int> IsAssetInPackage { get; init; }

    /* ---------- Hot Reload ---------- */
    public delegate* unmanaged[Stdcall]<NNGuid, void> MarkForReload { get; init; }
    public delegate* unmanaged[Stdcall]<void> ReloadMarkedAssets { get; init; }

    /* ---------- 統計 ---------- */
    public delegate* unmanaged[Stdcall]<ulong> GetLoadedAssetCount { get; init; }
    public delegate* unmanaged[Stdcall]<ulong> GetTotalMemoryUsage { get; init; }

    /* ---------- 初始化 ---------- */
    public delegate* unmanaged[Stdcall]<byte*, int> InitializeAssetManager { get; init; }
}

/// <summary>
/// 載入優先級枚舉（與 Native NNLoadPriority 對齊）。
/// </summary>
public enum LoadPriority
{
    Critical = 0,   /* 立即需要（UI 紋理等） */
    High = 1,       /* 相機附近資源 */
    Normal = 2,     /* 預設 */
    Low = 3,        /* 後台預載 */
    Background = 4  /* 最低優先級 */
}

/// <summary>
/// 資產載入完成回調委託（C# 側，與 Native NNAssetLoadCompletedCallback 對應）。
/// </summary>
[UnmanagedFunctionPointer(CallingConvention.StdCall)]
public delegate void AssetLoadCompletedCallback(ulong handle, int result, IntPtr userData);

/// <summary>
/// AssetHandle 擴展方法：便捷的非同步載入。
/// </summary>
public static class AssetHandleExtensions
{
    /// <summary>
    /// 異步載入資產，返回 TaskCompletionSource。
    /// 用法：
    ///   var tcs = AssetHandleExtensions.LoadAsync(guid, typeId);
    ///   var handle = await tcs.Task;
    /// </summary>
    public static unsafe TaskCompletionSource<ulong> LoadAsync(
        GUID guid,
        ulong typeId = 0,
        LoadPriority priority = LoadPriority.Normal)
    {
        var tcs = new TaskCompletionSource<ulong>();
        var api = NativeApiProvider.AssetManagerApi;

        if (api.LoadAssetAsync == null)
        {
            tcs.SetException(new InvalidOperationException("LoadAssetAsync 未接線"));
            return tcs;
        }

        /* 建立 GCHandle 防止回調被 GC */
        var callback = new AssetLoadCompletedCallback((handle, result, _) =>
        {
            if (result == 0)
                tcs.SetResult(handle);
            else
                tcs.SetException(new InvalidOperationException($"資產載入失敗: {guid.ToUuidString()}"));
        });

        var gcHandle = GCHandle.Alloc(callback);

        api.LoadAssetAsync(
            guid.ToNative(),
            typeId,
            (int)priority,
            /* TODO: 需要將委託轉為 Native 函數指標 */
            null,
            IntPtr.Zero.ToPointer());

        return tcs;
    }

    /// <summary>同步載入資產。</summary>
    public static unsafe AssetHandle LoadSync(GUID guid, ulong typeId = 0)
    {
        var api = NativeApiProvider.AssetManagerApi;
        if (api.LoadAssetSync == null)
            return AssetHandle.Zero;

        var raw = api.LoadAssetSync(guid.ToNative(), typeId);
        return new AssetHandle(raw);
    }

    /// <summary>依 GUID 取得已載入資產的 Handle。</summary>
    public static unsafe AssetHandle GetLoaded(GUID guid)
    {
        var api = NativeApiProvider.AssetManagerApi;
        if (api.GetAssetByGuid == null)
            return AssetHandle.Zero;

        var raw = api.GetAssetByGuid(guid.ToNative());
        return new AssetHandle(raw);
    }

    /// <summary>卸載資產。</summary>
    public static unsafe void Unload(AssetHandle handle)
    {
        if (handle.IsZero) return;
        if (NativeApiProvider.AssetManagerApi.UnloadAsset != null) NativeApiProvider.AssetManagerApi.UnloadAsset(handle.Value);
    }

    /// <summary>卸載資產（按 GUID）。</summary>
    public static unsafe void Unload(GUID guid)
    {
        if (NativeApiProvider.AssetManagerApi.UnloadAssetByGuid != null) NativeApiProvider.AssetManagerApi.UnloadAssetByGuid(guid.ToNative());
    }

    /* ========== Hot Reload ========== */

    /// <summary>標記資產需要重載（Editor 通知 Runtime 用）。</summary>
    public static unsafe void MarkForReload(GUID guid)
    {
        var api = NativeApiProvider.AssetManagerApi;
        if (api.MarkForReload != null)
            api.MarkForReload(guid.ToNative());
    }

    /// <summary>重新載入所有被標記的資產。</summary>
    public static unsafe void ReloadMarkedAssets()
    {
        var api = NativeApiProvider.AssetManagerApi;
        if (api.ReloadMarkedAssets != null)
            api.ReloadMarkedAssets();
    }
}
