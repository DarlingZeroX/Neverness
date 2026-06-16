namespace Neverness.Editor.Assets;

/// <summary>
/// 资产导入器接口。
///
/// 每种文件类型对应一个 importer 实现。Importer 职责：
///   - 读取/解析 source asset
///   - 转换为 engine 内部数据格式
///   - 收集依赖关系
///   - 生成 blob 数据供编译器序列化为 .nnasset
///
/// Importer 通过 [AssetImporter] attribute 或 SupportedExtensions 属性声明支持的扩展名。
/// </summary>
public interface IAssetImporter
{
    /// <summary>此 importer 支持的文件扩展名（含点号，小写）。</summary>
    string[] SupportedExtensions { get; }

    /// <summary>显示名称（用于 UI）。</summary>
    string DisplayName { get; }

    /// <summary>导入 source asset，返回可编译的中间数据。</summary>
    ImportResult Import(AssetImportContext context);
}

/// <summary>
/// 可选接口：支持自定义导入设置的 importer。
/// </summary>
public interface ISettingsAwareImporter : IAssetImporter
{
    /// <summary>返回此 importer 的默认设置字典。</summary>
    Dictionary<string, string> GetDefaultSettings();

    /// <summary>验证设置值是否合法。</summary>
    bool ValidateSettings(Dictionary<string, string> settings, out string? errorMessage);
}
