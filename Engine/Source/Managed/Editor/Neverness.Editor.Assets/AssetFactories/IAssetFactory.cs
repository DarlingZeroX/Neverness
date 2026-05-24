using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets.AssetFactories;

/// <summary>
/// 资产工厂接口——每个实现类代表一种可创建的资产类型。
/// 通过 <see cref="AssetFactoryRegistry"/> 自动发现和注册。
/// </summary>
public interface IAssetFactory
{
    /// <summary>资产类型的显示名称（如 "Scene", "Material"）。</summary>
    string DisplayName { get; }

    /// <summary>资产分类（如 "Scene", "Rendering", "Scripting"），用于菜单分组。</summary>
    string Category { get; }

    /// <summary>FontAwesome 图标常量。</summary>
    string Icon { get; }

    /// <summary>资产文件扩展名（如 ".scene", ".material"），含点号。</summary>
    string FileExtension { get; }

    /// <summary>在指定目录创建资产文件。</summary>
    /// <param name="directoryPath">目标目录的绝对路径。</param>
    /// <returns>创建成功返回实际文件路径，失败返回 null。</returns>
    NPath? CreateAsset(NPath directoryPath);
}
