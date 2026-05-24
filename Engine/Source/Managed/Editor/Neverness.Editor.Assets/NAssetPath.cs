namespace Neverness.Editor.Assets;

/// <summary>
/// Editor 逻辑路径类型——表示 Content Browser 中的资产逻辑路径。
///
/// 设计原则：
///   - immutable，构造时 normalize 一次
///   - 不含盘符、不含 URI scheme、不含 OS 根
///   - 统一 `/` 分隔符，折叠双斜杠，去前导 `/`
///   - 用于 Editor 显示、GUID 映射、ContentBrowser 导航
///   - 相等性基于 OrdinalIgnoreCase
///
/// 示例：
///   Scenes/Main.scene
///   Textures/UI/Button.png
///   Characters/Hero/Hero.prefab
/// </summary>
public readonly record struct NAssetPath : IEquatable<NAssetPath>
{
    /// <summary>规范化后的逻辑路径字符串。</summary>
    public string Value { get; }

    /// <summary>空路径单例。</summary>
    public static readonly NAssetPath Empty = new(string.Empty);

    public NAssetPath(string path)
    {
        Value = Normalize(path);
    }

    /* ======================== 属性 ======================== */

    /// <summary>文件名含扩展名（如 "Main.scene"）。</summary>
    public string FileName
    {
        get
        {
            if (string.IsNullOrEmpty(Value)) return string.Empty;
            var idx = Value.LastIndexOf('/');
            return idx >= 0 ? Value[(idx + 1)..] : Value;
        }
    }

    /// <summary>扩展名含点号（如 ".scene"）。无扩展名返回空。</summary>
    public string Extension
    {
        get
        {
            var name = FileName;
            var idx = name.LastIndexOf('.');
            return idx >= 0 ? name[idx..] : string.Empty;
        }
    }

    /// <summary>不含扩展名的文件名（如 "Main"）。</summary>
    public string FileNameWithoutExtension
    {
        get
        {
            var name = FileName;
            var idx = name.LastIndexOf('.');
            return idx >= 0 ? name[..idx] : name;
        }
    }

    /// <summary>父目录 asset path。根级别返回空。</summary>
    public NAssetPath Parent
    {
        get
        {
            if (string.IsNullOrEmpty(Value)) return Empty;
            var idx = Value.LastIndexOf('/');
            return idx >= 0 ? new NAssetPath(Value[..idx]) : Empty;
        }
    }

    /// <summary>是否为空路径。</summary>
    public bool IsEmpty => string.IsNullOrEmpty(Value);

    /* ======================== 操作 ======================== */

    /// <summary>拼接子路径段。</summary>
    public NAssetPath Combine(string segment)
    {
        if (string.IsNullOrEmpty(Value)) return new NAssetPath(segment);
        var seg = segment.TrimStart('/');
        return new NAssetPath($"{Value}/{seg}");
    }

    /* ======================== 规范化 ======================== */

    /// <summary>
    /// 规范化 asset path：统一 `/`、折叠双斜杠、去前导 `/`。
    /// 不调用 Path.GetFullPath（不是 OS 路径）。
    /// </summary>
    public static string Normalize(string path)
    {
        if (string.IsNullOrEmpty(path)) return string.Empty;

        // 统一分隔符
        path = path.Replace('\\', '/');

        // 折叠连续双斜杠
        while (path.Contains("//", StringComparison.Ordinal))
            path = path.Replace("//", "/", StringComparison.Ordinal);

        // 去前导 /
        path = path.TrimStart('/');

        // 去尾部 /（目录路径保留为空时由 IsEmpty 判断）
        path = path.TrimEnd('/');

        return path;
    }

    /* ======================== 相等性 ======================== */

    public bool Equals(NAssetPath other) =>
        string.Equals(Value, other.Value, StringComparison.OrdinalIgnoreCase);

    public override int GetHashCode() =>
        StringComparer.OrdinalIgnoreCase.GetHashCode(Value ?? string.Empty);

    /* ======================== 显示 ======================== */

    public override string ToString() => Value;
}
