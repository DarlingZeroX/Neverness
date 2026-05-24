using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 构建配置。
///
/// 定义一次构建的完整参数。
/// </summary>
public sealed class BuildProfile
{
    /// <summary>配置名称。</summary>
    public string Name { get; set; } = "Default";

    /// <summary>构建目标平台。</summary>
    public BuildTarget Target { get; set; } = BuildTarget.Windows;

    /// <summary>Assets 根目录。</summary>
    public NPath AssetsRoot { get; set; }

    /// <summary>Library 根目录。</summary>
    public NPath LibraryRoot { get; set; }

    /// <summary>输出根目录。</summary>
    public NPath OutputRoot { get; set; }

    /// <summary>压缩方式。</summary>
    public CompressionMethod Compression { get; set; } = CompressionMethod.Zstd;

    /// <summary>压缩级别。</summary>
    public int CompressionLevel { get; set; } = 3;

    /// <summary>是否剔除未引用资产。</summary>
    public bool StripUnusedAssets { get; set; } = true;

    /// <summary>是否增量构建。</summary>
    public bool IncrementalBuild { get; set; } = true;

    /// <summary>分组配置。</summary>
    public AssetGroupManager? GroupManager { get; set; }

    /// <summary>创建 Editor 构建配置。</summary>
    public static BuildProfile CreateEditor(NPath projectRoot)
    {
        return new BuildProfile
        {
            Name = "Editor",
            AssetsRoot = projectRoot.Combine("Assets"),
            LibraryRoot = projectRoot.Combine("Library"),
            OutputRoot = projectRoot.Combine("Build/Editor"),
            Compression = CompressionMethod.None,
            StripUnusedAssets = false,
            IncrementalBuild = true
        };
    }

    /// <summary>创建 Release 构建配置。</summary>
    public static BuildProfile CreateRelease(NPath projectRoot, BuildTarget target = BuildTarget.Windows)
    {
        return new BuildProfile
        {
            Name = "Release",
            Target = target,
            AssetsRoot = projectRoot.Combine("Assets"),
            LibraryRoot = projectRoot.Combine("Library"),
            OutputRoot = projectRoot.Combine($"Build/{target}"),
            Compression = CompressionMethod.Zstd,
            CompressionLevel = 9,
            StripUnusedAssets = true,
            IncrementalBuild = false
        };
    }
}
