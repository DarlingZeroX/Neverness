namespace Neverness.Editor.Assets;

/// <summary>
/// 标记 importer 类支持的文件扩展名。
///
/// 用法：
///   [AssetImporter(".png", ".jpg", ".tga")]
///   public class TextureImporter : IAssetImporter { ... }
///
/// ImporterRegistry.Discover() 通过反射扫描所有标记此 attribute 的类。
/// </summary>
[AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
public sealed class AssetImporterAttribute : Attribute
{
    /// <summary>支持的扩展名（含点号，自动转小写）。</summary>
    public string[] Extensions { get; }

    /// <summary>可选优先级（数值越高越优先，默认 0）。</summary>
    public int Priority { get; set; }

    public AssetImporterAttribute(params string[] extensions)
    {
        Extensions = extensions.Select(e => e.ToLowerInvariant()).ToArray();
    }
}
