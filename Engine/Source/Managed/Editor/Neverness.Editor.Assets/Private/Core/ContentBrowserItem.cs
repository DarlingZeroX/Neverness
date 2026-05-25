using Neverness.Editor.Assets;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets.Private.Core;

public abstract class ContentItem
{
    public bool Selected { get; set; }
    public bool Renaming { get; set; }
    public string Name = string.Empty; 
    public int UIFlags { get; set; }

    public NPath AssetPath { get; set; } = NPath.Empty;

    public NVirtualPath Path { get; set; } = default;
    public string Extension { get; set; } = string.Empty;
}

public sealed class ContentFile : ContentItem
{
    public string AssetType { get; init; } = "Unknown";
}

public sealed class ContentDirectory : ContentItem
{
    public List<ContentDirectory> Directories { get; } = [];

    public List<ContentFile> Files { get; } = [];

    public IEnumerable<ContentItem> Children
    {
        get
        {
            foreach (var dir in Directories)
                yield return dir;

            foreach (var file in Files)
                yield return file;
        }
    }
}

