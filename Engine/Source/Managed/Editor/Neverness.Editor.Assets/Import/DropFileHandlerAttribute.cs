namespace Neverness.Editor.Assets;

/// <summary>
/// 标记类为拖放文件处理器。扩展名在此声明，供 <see cref="DropImportService"/> 反射发现。
/// 对标 <see cref="AssetImporterAttribute"/> 模式。
/// </summary>
[AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
public sealed class DropFileHandlerAttribute : Attribute
{
    /// <summary>支持的扩展名（含点号，如 ".png"）。空数组 = 兜底处理器。</summary>
    public string[] Extensions { get; }

    public DropFileHandlerAttribute(params string[] extensions)
    {
        Extensions = extensions.Length > 0
            ? extensions.Select(e => e.ToLowerInvariant()).ToArray()
            : Array.Empty<string>();
    }
}
