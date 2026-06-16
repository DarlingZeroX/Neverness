using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产导入上下文。
///
/// 在 Import 阶段传递给 IAssetImporter.Import()，提供：
///   - 源文件路径和元数据
///   - 资产 GUID
///   - 输出路径（Library/Imported/XX/guid.nnasset）
///   - 从 .meta 读取的导入设置
///   - 依赖收集器
/// </summary>
public sealed class AssetImportContext
{
    /// <summary>源资产文件完整路径。</summary>
    public NPath SourceAssetPath { get; init; }

    /// <summary>.meta 文件完整路径。</summary>
    public NPath MetaFilePath { get; init; }

    /// <summary>资产 GUID（来自 .meta）。</summary>
    public GUID AssetGuid { get; init; }

    /// <summary>资产的虚拟路径（/assets/...）。</summary>
    public NVirtualPath VirtualPath { get; init; }

    /// <summary>.nnasset 输出完整路径。</summary>
    public NPath OutputPath { get; init; }

    /// <summary>Library 根目录。</summary>
    public NPath LibraryRoot { get; init; }

    /// <summary>从 .meta 读取的导入设置。</summary>
    public Dictionary<string, string> ImportSettings { get; init; } = new();

    /// <summary>依赖收集器。</summary>
    public DependencyCollector Dependencies { get; } = new();

    /// <summary>读取源文件全部字节。</summary>
    public byte[] ReadAllBytes()
    {
        return File.ReadAllBytes(SourceAssetPath.FullPath);
    }

    /// <summary>打开源文件的读取流。</summary>
    public Stream OpenReadStream()
    {
        return File.OpenRead(SourceAssetPath.FullPath);
    }

    /// <summary>源文件扩展名（小写，含点号）。</summary>
    public string Extension => SourceAssetPath.Extension.ToLowerInvariant();

    /// <summary>源文件名（不含路径）。</summary>
    public string FileName => SourceAssetPath.FileName;

    /// <summary>从导入设置中获取值，不存在返回默认值。</summary>
    public string GetSetting(string key, string defaultValue = "")
    {
        return ImportSettings.TryGetValue(key, out var value) ? value : defaultValue;
    }

    /// <summary>从导入设置中获取布尔值。</summary>
    public bool GetSettingBool(string key, bool defaultValue = false)
    {
        if (!ImportSettings.TryGetValue(key, out var value))
            return defaultValue;
        return value.Equals("true", StringComparison.OrdinalIgnoreCase)
            || value == "1";
    }

    /// <summary>从导入设置中获取整数值。</summary>
    public int GetSettingInt(string key, int defaultValue = 0)
    {
        if (!ImportSettings.TryGetValue(key, out var value) || !int.TryParse(value, out var result))
            return defaultValue;
        return result;
    }

    /// <summary>从导入设置中获取浮点值。</summary>
    public float GetSettingFloat(string key, float defaultValue = 0f)
    {
        if (!ImportSettings.TryGetValue(key, out var value) || !float.TryParse(value, out var result))
            return defaultValue;
        return result;
    }
}

/// <summary>
/// 依赖收集器。
///
/// Importer 在 Import 过程中通过 AddDependency() 声明依赖的其他资产 GUID。
/// </summary>
public sealed class DependencyCollector
{
    private readonly List<GUID> _dependencies = new();

    /// <summary>已收集的依赖 GUID 列表。</summary>
    public IReadOnlyList<GUID> Dependencies => _dependencies;

    /// <summary>添加一个依赖 GUID。</summary>
    public void Add(GUID guid)
    {
        if (!guid.IsZero && !_dependencies.Contains(guid))
            _dependencies.Add(guid);
    }

    /// <summary>通过虚拟路径添加依赖（自动查找 GUID）。</summary>
    public void AddByPath(NVirtualPath virtualPath)
    {
        if (EditorAssetDatabase.TryGetGuid(virtualPath, out var guid))
            Add(guid);
    }

    /// <summary>通过字符串虚拟路径添加依赖（Importer 便利重载）。</summary>
    public void AddByPath(string virtualPath) => AddByPath(new NVirtualPath(virtualPath));

    /// <summary>添加多个依赖。</summary>
    public void AddRange(IEnumerable<GUID> guids)
    {
        foreach (var guid in guids)
            Add(guid);
    }
}
