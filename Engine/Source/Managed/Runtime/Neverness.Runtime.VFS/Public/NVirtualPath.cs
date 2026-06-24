namespace Neverness.Runtime.VFS;

/// <summary>
/// Runtime VFSService 路径类型——表示虚拟文件系统路径。
///
/// 设计原则：
///   - immutable，构造时 normalize 一次
///   - 统一 `/` 分隔符，折叠双斜杠
///   - 支持 URI scheme 检测（assets://, engine://, memory:// 等）
///   - Runtime 永远只接触此类型，不接触 NPath（OS 路径，同模块）
///   - 相等性基于 OrdinalIgnoreCase
///
/// 示例：
///   /assets/hero.png          → scheme="assets", stored="/assets/hero.png"
///   assets://hero.png         → scheme="assets", stored="assets://hero.png"
///   memory://runtime/mesh     → scheme="memory"
///   /engine/textures/white    → scheme="engine"
/// </summary>
public readonly record struct NVirtualPath : IEquatable<NVirtualPath>
{
    /// <summary>规范化后的完整路径字符串。</summary>
    public string FullPath { get; }

    /* ======================== 内置根路径 ======================== */

    public static readonly NVirtualPath AssetsRoot = new("/assets/");
    public static readonly NVirtualPath EngineRoot = new("/engine/");
    public static readonly NVirtualPath MemoryRoot = new("memory:///");

    /* ======================== 构造 ======================== */

    public NVirtualPath(string path)
    {
        FullPath = Normalize(path);
    }

    /* ======================== 属性 ======================== */

    /// <summary>
    /// URI scheme（如 "assets", "engine", "memory"）。
    /// 检测 `://` 前缀或已知根路径前缀。
    /// 无法推断时返回空字符串。
    /// </summary>
    public string Scheme
    {
        get
        {
            if (string.IsNullOrEmpty(FullPath)) return string.Empty;

            // 显式 scheme（如 "assets://..."）
            var idx = FullPath.IndexOf("://", StringComparison.Ordinal);
            if (idx > 0) return FullPath[..idx];

            // 已知根路径前缀（如 "/assets/" → "assets"）
            if (FullPath.StartsWith("/assets/", StringComparison.OrdinalIgnoreCase)) return "assets";
            if (FullPath.StartsWith("/engine/", StringComparison.OrdinalIgnoreCase)) return "engine";
            if (FullPath.StartsWith("/projectIntermediate/", StringComparison.OrdinalIgnoreCase)) return "intermediate";
            if (FullPath.StartsWith("/projectSettings/", StringComparison.OrdinalIgnoreCase)) return "settings";

            return string.Empty;
        }
    }

    /// <summary>文件名含扩展名。</summary>
    public string FileName
    {
        get
        {
            var path = StripScheme(FullPath);
            // 去掉尾部 /
            path = path.TrimEnd('/');
            var idx = path.LastIndexOf('/');
            return idx >= 0 ? path[(idx + 1)..] : path;
        }
    }

    /// <summary>扩展名含点号（如 ".png"）。无扩展名返回空。</summary>
    public string Extension
    {
        get
        {
            var name = FileName;
            var idx = name.LastIndexOf('.');
            return idx >= 0 ? name[idx..] : string.Empty;
        }
    }

    /// <summary>不含扩展名的文件名。</summary>
    public string FileNameWithoutExtension
    {
        get
        {
            var name = FileName;
            var idx = name.LastIndexOf('.');
            return idx >= 0 ? name[..idx] : name;
        }
    }

    /// <summary>父目录 virtual path。</summary>
    public NVirtualPath Parent
    {
        get
        {
            var stripped = StripScheme(FullPath);
            var trimmed = stripped.TrimEnd('/');
            var idx = trimmed.LastIndexOf('/');
            if (idx < 0) return this;

            var parentPath = trimmed[..(idx + 1)]; // 保留尾部 /
            var scheme = Scheme;
            return string.IsNullOrEmpty(scheme)
                ? new NVirtualPath(parentPath)
                : new NVirtualPath($"{scheme}://{parentPath}");
        }
    }

    /// <summary>是否为空路径。</summary>
    public bool IsEmpty => string.IsNullOrEmpty(FullPath);

    /* ======================== 操作 ======================== */

    /// <summary>拼接子路径段。</summary>
    public NVirtualPath Combine(string segment)
    {
        var baseStr = FullPath.TrimEnd('/');
        var seg = segment.TrimStart('/');
        return new NVirtualPath($"{baseStr}/{seg}");
    }

    /* ======================== 工厂 ======================== */

    /// <summary>从显式 scheme 和路径创建。</summary>
    public static NVirtualPath FromScheme(string scheme, string path)
    {
        var normalized = Normalize(path);
        return new NVirtualPath($"{scheme}://{normalized}");
    }

    /* ======================== 规范化 ======================== */

    /// <summary>
    /// 规范化 virtual path：统一 `/`、折叠双斜杠。
    /// 不调用 Path.GetFullPath（不是 OS 路径）。
    /// </summary>
    public static string Normalize(string path)
    {
        if (string.IsNullOrEmpty(path)) return string.Empty;

        // 统一分隔符
        path = path.Replace('\\', '/');

        // 折叠连续双斜杠，但保留 scheme 后的 `://`
        // 先保护 `://`
        path = path.Replace("://", "__SCHEME_COLON__", StringComparison.Ordinal);

        while (path.Contains("//", StringComparison.Ordinal))
            path = path.Replace("//", "/", StringComparison.Ordinal);

        // 恢复 `://`
        path = path.Replace("__SCHEME_COLON__", "://", StringComparison.Ordinal);

        return path;
    }

    /* ======================== 内部 ======================== */

    private static string StripScheme(string fullPath)
    {
        var idx = fullPath.IndexOf("://", StringComparison.Ordinal);
        return idx >= 0 ? fullPath[(idx + 3)..] : fullPath;
    }

    /* ======================== 相等性 ======================== */

    public bool Equals(NVirtualPath other) =>
        string.Equals(FullPath, other.FullPath, StringComparison.OrdinalIgnoreCase);

    public override int GetHashCode() =>
        StringComparer.OrdinalIgnoreCase.GetHashCode(FullPath ?? string.Empty);

    /* ======================== 显示 ======================== */

    public override string ToString() => FullPath;
}
