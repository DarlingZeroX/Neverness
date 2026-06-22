using Avalonia.Media;
using static Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ContentBrowserColors;

namespace Neverness.Editor.AvaloniaFrontend.Views.ContentBrowser.ThumbnailGrid;

/// <summary>
/// 缩略图文件信息提供器——根据扩展名/资产类型返回图标、标签和徽章颜色。
/// </summary>
internal static class ThumbnailFileInfoProvider
{
    /// <summary>根据文件扩展名和资产类型获取显示信息。</summary>
    internal static (string Icon, string Label, IBrush BadgeColor) GetInfo(string extension, string? assetType)
    {
        return extension?.ToLower() switch
        {
            ".png" or ".jpg" or ".jpeg" or ".bmp" or ".tga" or ".hdr"
                => ("🖼️", "TEX", BadgeTexture),
            ".fbx" or ".obj" or ".gltf" or ".glb"
                => ("🧊", "MESH", BadgeDefault),
            ".wav" or ".mp3" or ".ogg"
                => ("🎵", "AUD", BadgeAudio),
            ".mp4" or ".avi" or ".mov"
                => ("🎬", "VID", BadgeAudio),
            ".cs" or ".cpp" or ".h" or ".py"
                => ("📝", "CODE", BadgeScript),
            ".json" or ".xml" or ".yaml" or ".yml"
                => ("📋", "CFG", BadgeDefault),
            ".txt" or ".md"
                => ("📄", "TXT", BadgeDefault),
            ".shader" or ".hlsl" or ".glsl"
                => ("☀️", "SHD", BadgeMaterial),
            ".scene"
                => ("🗺️", "SCENE", BadgeScene),
            ".prefab"
                => ("🧊", "PREFB", BadgePrefab),
            ".mat"
                => ("📋", "MATER", BadgeMaterial),
            ".html"
                => ("🌐", "HTML", BadgeDefault),
            ".lua"
                => ("📜", "LUA", BadgeScript),
            _ => ("📄", "FILE", BadgeDefault)
        };
    }
}
