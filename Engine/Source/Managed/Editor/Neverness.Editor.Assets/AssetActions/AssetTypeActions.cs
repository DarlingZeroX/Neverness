namespace Neverness.Editor.Assets.AssetActions;

/// <summary>
/// 资产类型操作元数据——描述一种资产类型在编辑器中的行为。
/// </summary>
public sealed class AssetTypeActions
{
    /// <summary>资产类型显示名称。</summary>
    public required string DisplayName { get; init; }

    /// <summary>文件扩展名（含点号，如 ".scene"）。</summary>
    public required string FileExtension { get; init; }

    /// <summary>FontAwesome 图标。</summary>
    public string Icon { get; init; } = string.Empty;

    /// <summary>资产分类。</summary>
    public string Category { get; init; } = string.Empty;

    /// <summary>是否支持双击打开。</summary>
    public bool CanOpen { get; init; }

    /// <summary>是否支持导入。</summary>
    public bool CanImport { get; init; }
}
