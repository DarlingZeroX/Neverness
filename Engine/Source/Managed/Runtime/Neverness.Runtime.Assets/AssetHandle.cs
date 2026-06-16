using System.Runtime.InteropServices;

namespace Neverness.Runtime.Assets;

/// <summary>
/// 类型化资产 Handle，blittable，可跨 P/Invoke 传递。
///
/// 迁移后：所有方法直接调用 C# <see cref="AssetManager.Instance"/>，
/// 不再经过 NativeApiProvider / C++ ABI。
/// </summary>
/// <typeparam name="T">资产类型（仅用于类型安全，不影响内存布局）。</typeparam>
[StructLayout(LayoutKind.Sequential)]
public readonly struct AssetHandle<T> : IEquatable<AssetHandle<T>>
{
    /// <summary>原始值（HandleTable 索引 + generation 编码）。</summary>
    public ulong Value { get; }

    /// <summary>是否为零（无效 Handle）。</summary>
    public bool IsZero => Value == 0;

    /// <summary>全零无效 Handle。</summary>
    public static AssetHandle<T> Zero => default;

    public AssetHandle(ulong value)
    {
        Value = value;
    }

    /* ========== 查询 ========== */

    /// <summary>资产是否已加载。</summary>
    public bool IsLoaded => !IsZero && AssetManager.Instance.IsLoaded(Value);

    /// <summary>资产是否正在加载中。</summary>
    public bool IsLoading => !IsZero && AssetManager.Instance.IsLoading(Value);

    /// <summary>取得资产 GUID。</summary>
    public GUID GetGuid()
    {
        return IsZero ? GUID.Zero : AssetManager.Instance.GetGuidByHandle(Value);
    }

    /* ========== 引用计数 ========== */

    /// <summary>增加引用计数。</summary>
    public void AddRef()
    {
        if (!IsZero) AssetManager.Instance.AddRef(Value);
    }

    /// <summary>减少引用计数（计数归零时自动卸载）。</summary>
    public void Release()
    {
        if (!IsZero) AssetManager.Instance.ReleaseRef(Value);
    }

    /// <summary>取得当前引用计数。</summary>
    public uint GetRefCount()
    {
        return IsZero ? 0u : AssetManager.Instance.GetRefCount(Value);
    }

    /* ========== 数据存取 ========== */

    /// <summary>取得资产原始数据大小（字节）。</summary>
    public long GetDataSize()
    {
        return IsZero ? 0L : AssetManager.Instance.GetAssetDataSize(Value);
    }

    /// <summary>取得资产中 blob 数量。</summary>
    public int GetBlobCount()
    {
        return IsZero ? 0 : AssetManager.Instance.GetBlobCount(Value);
    }

    /// <summary>取得指定 blob 的大小。</summary>
    public long GetBlobSize(int index)
    {
        return IsZero ? 0L : AssetManager.Instance.GetBlobSize(Value, index);
    }

    /// <summary>取得资产原始数据（Span 切片，零拷贝）。</summary>
    public ReadOnlySpan<byte> GetAssetData()
    {
        return IsZero ? ReadOnlySpan<byte>.Empty : AssetManager.Instance.GetAssetData(Value);
    }

    /// <summary>取得指定 blob 的数据（Span 切片，零拷贝）。</summary>
    public ReadOnlySpan<byte> GetBlobData(int index)
    {
        return IsZero ? ReadOnlySpan<byte>.Empty : AssetManager.Instance.GetBlobData(Value, index);
    }

    /* ========== 运算子 ========== */

    public bool Equals(AssetHandle<T> other) => Value == other.Value;
    public override bool Equals(object? obj) => obj is AssetHandle<T> h && Equals(h);
    public override int GetHashCode() => Value.GetHashCode();
    public override string ToString() => IsZero ? "AssetHandle(Zero)" : $"AssetHandle(0x{Value:x16})";

    public static bool operator ==(AssetHandle<T> left, AssetHandle<T> right) => left.Equals(right);
    public static bool operator !=(AssetHandle<T> left, AssetHandle<T> right) => !left.Equals(right);

    /// <summary>从原始值构造。</summary>
    public static AssetHandle<T> FromRaw(ulong value) => new(value);
}

/// <summary>
/// 非类型化资产 Handle（简化用）。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public readonly struct AssetHandle : IEquatable<AssetHandle>
{
    public ulong Value { get; }
    public bool IsZero => Value == 0;
    public static AssetHandle Zero => default;

    public AssetHandle(ulong value) => Value = value;

    public bool IsLoaded => !IsZero && AssetManager.Instance.IsLoaded(Value);
    public bool IsLoading => !IsZero && AssetManager.Instance.IsLoading(Value);

    public GUID GetGuid()
    {
        return IsZero ? GUID.Zero : AssetManager.Instance.GetGuidByHandle(Value);
    }

    public void AddRef()
    {
        if (!IsZero) AssetManager.Instance.AddRef(Value);
    }

    public void Release()
    {
        if (!IsZero) AssetManager.Instance.ReleaseRef(Value);
    }

    public uint GetRefCount()
    {
        return IsZero ? 0u : AssetManager.Instance.GetRefCount(Value);
    }

    public long GetDataSize()
    {
        return IsZero ? 0L : AssetManager.Instance.GetAssetDataSize(Value);
    }

    public int GetBlobCount()
    {
        return IsZero ? 0 : AssetManager.Instance.GetBlobCount(Value);
    }

    public ReadOnlySpan<byte> GetAssetData()
    {
        return IsZero ? ReadOnlySpan<byte>.Empty : AssetManager.Instance.GetAssetData(Value);
    }

    public bool Equals(AssetHandle other) => Value == other.Value;
    public override bool Equals(object? obj) => obj is AssetHandle h && Equals(h);
    public override int GetHashCode() => Value.GetHashCode();
    public override string ToString() => IsZero ? "AssetHandle(Zero)" : $"AssetHandle(0x{Value:x16})";

    public static bool operator ==(AssetHandle left, AssetHandle right) => left.Equals(right);
    public static bool operator !=(AssetHandle left, AssetHandle right) => !left.Equals(right);

    /* ========== 静态加载便捷方法 ========== */

    /// <summary>同步加载资产。</summary>
    public static AssetHandle LoadSync(GUID guid, ulong typeId = 0)
    {
        if (guid.IsZero) return Zero;
        var raw = AssetManager.Instance.LoadAssetSync(guid, typeId);
        return new AssetHandle(raw);
    }

    /// <summary>异步加载资产。</summary>
    public static async ValueTask<ulong> LoadAsync(GUID guid, ulong typeId = 0,
        LoadPriority priority = LoadPriority.Normal, CancellationToken ct = default)
    {
        if (guid.IsZero) return 0;
        return await AssetManager.Instance.LoadAssetAsync(guid, typeId, priority, ct).ConfigureAwait(false);
    }

    /// <summary>依 GUID 取得已加载资产的 Handle。</summary>
    public static AssetHandle GetLoaded(GUID guid)
    {
        if (guid.IsZero) return Zero;
        var raw = AssetManager.Instance.GetLoadedAsset(guid);
        return new AssetHandle(raw);
    }

    /// <summary>卸载资产（按 Handle）。</summary>
    public static void Unload(AssetHandle handle)
    {
        if (!handle.IsZero) AssetManager.Instance.UnloadAsset(handle.Value);
    }

    /// <summary>卸载资产（按 GUID）。</summary>
    public static void Unload(GUID guid)
    {
        if (!guid.IsZero) AssetManager.Instance.UnloadAssetByGuid(guid);
    }

    /* ========== Hot Reload ========== */

    /// <summary>标记资产需要重载（Editor 通知 Runtime 用）。</summary>
    public static void MarkForReload(GUID guid)
    {
        if (!guid.IsZero) AssetManager.Instance.MarkForReload(guid);
    }

    /// <summary>重新加载所有被标记的资产。</summary>
    public static void ReloadMarkedAssets()
    {
        AssetManager.Instance.ReloadMarkedAssets();
    }
}

/// <summary>
/// 载入优先级枚举（与 C++ NNLoadPriority 对齐）。
/// </summary>
public enum LoadPriority
{
    Critical = 0,   /* 立即需要（UI 纹理等） */
    High = 1,       /* 相机附近资源 */
    Normal = 2,     /* 默认 */
    Low = 3,        /* 后台预载 */
    Background = 4  /* 最低优先级 */
}

/// <summary>AssetHandle 类型转换辅助。</summary>
public static class AssetHandleCastExtensions
{
    /// <summary>将非泛型 Handle 转为泛型 Handle（零拷贝，仅类型转换）。</summary>
    public static AssetHandle<T> As<T>(this AssetHandle handle)
    {
        return new AssetHandle<T>(handle.Value);
    }
}
