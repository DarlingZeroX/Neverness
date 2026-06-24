namespace Neverness.Runtime.VFS;

/// <summary>
/// OS 磁盘路径类型——表示真实文件系统路径。
///
/// 设计原则：
///   - immutable，构造时 normalize 一次
///   - 统一 `/` 分隔符，折叠双斜杠
///   - 调用 Path.GetFullPath 解析相对路径
///   - 相等性基于 OrdinalIgnoreCase
/// </summary>
public readonly record struct NPath : IEquatable<NPath>
{
    /// <summary>规范化后的完整路径（统一 `/`）。</summary>
    public string FullPath { get; }

    /// <summary>空路径单例。</summary>
    public static readonly NPath Empty = new(string.Empty);

    public NPath(string path)
    {
        FullPath = Normalize(path);
    }

    /* ======================== 属性 ======================== */

    /// <summary>文件名含扩展名（如 "hero.png"）。</summary>
    public string FileName => Path.GetFileName(FullPath);

    /// <summary>扩展名含点号（如 ".png"）。</summary>
    public string Extension => Path.GetExtension(FullPath);

    /// <summary>不含扩展名的文件名（如 "hero"）。</summary>
    public string FileNameWithoutExtension => Path.GetFileNameWithoutExtension(FullPath);

    /// <summary>父目录路径。根目录返回自身。</summary>
    public NPath Parent
    {
        get
        {
            var dir = Path.GetDirectoryName(FullPath);
            return dir == null ? this : new NPath(dir);
        }
    }

    /// <summary>是否为空路径。</summary>
    public bool IsEmpty => string.IsNullOrEmpty(FullPath);

    /// <summary>路径指向的文件或目录是否存在。</summary>
    public bool Exists => File.Exists(FullPath) || Directory.Exists(FullPath);

    /// <summary>路径是否指向一个已存在的目录。</summary>
    public bool IsDirectory => Directory.Exists(FullPath);

    /* ======================== 操作 ======================== */

    /// <summary>拼接子路径。</summary>
    public NPath Combine(string segment) => new(Path.Combine(FullPath, segment));

    /// <summary>拼接多个子路径。</summary>
    public NPath Combine(params string[] segments)
    {
        var parts = new string[segments.Length + 1];
        parts[0] = FullPath;
        Array.Copy(segments, 0, parts, 1, segments.Length);
        return new NPath(Path.Combine(parts));
    }

    /// <summary>计算相对于 basePath 的相对路径。</summary>
    public string RelativeTo(NPath basePath) =>
        Path.GetRelativePath(basePath.FullPath, FullPath).Replace('\\', '/');

    /* ======================== 规范化 ======================== */

    /// <summary>规范化路径：统一 `/`、折叠双斜杠、GetFullPath。</summary>
    public static string Normalize(string path)
    {
        if (string.IsNullOrEmpty(path)) return string.Empty;

        // 统一分隔符
        path = path.Replace('\\', '/');

        // 折叠连续双斜杠
        while (path.Contains("//", StringComparison.Ordinal))
            path = path.Replace("//", "/", StringComparison.Ordinal);

        // 解析为绝对路径（处理 .. 和 .）
        // GetFullPath 在 Windows 会使用 \，需再次统一
        try
        {
            path = Path.GetFullPath(path).Replace('\\', '/');
        }
        catch
        {
            // 路径格式不合法时保留原值（如 URI 风格）
        }

        return path;
    }

    /* ======================== 相等性 ======================== */

    public bool Equals(NPath other) =>
        string.Equals(FullPath, other.FullPath, StringComparison.OrdinalIgnoreCase);

    public override int GetHashCode() =>
        StringComparer.OrdinalIgnoreCase.GetHashCode(FullPath ?? string.Empty);

    /* ======================== 显示 ======================== */

    public override string ToString() => FullPath;
}
