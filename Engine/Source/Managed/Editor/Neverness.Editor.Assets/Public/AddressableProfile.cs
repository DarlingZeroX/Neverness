namespace Neverness.Editor.Assets;

/// <summary>
/// Addressable 构建配置。
///
/// 定义不同构建场景的参数：
///   - Editor（开发模式，直接从 Library/Imported 加载）
///   - Development（调试构建，未压缩或低压缩）
///   - Release（发布构建，高压缩，剔除未引用资产）
/// </summary>
public sealed class AddressableProfile
{
    /// <summary>配置名称。</summary>
    public string Name { get; set; } = "Default";

    /// <summary>构建目标平台。</summary>
    public BuildTarget Target { get; set; } = BuildTarget.Windows;

    /// <summary>输出目录（相对项目根目录）。</summary>
    public string OutputPath { get; set; } = "Build";

    /// <summary>是否压缩。</summary>
    public bool Compress { get; set; } = true;

    /// <summary>压缩算法。</summary>
    public CompressionMethod CompressionMethod { get; set; } = CompressionMethod.Zstd;

    /// <summary>压缩级别（1-22 for Zstd）。</summary>
    public int CompressionLevel { get; set; } = 3;

    /// <summary>是否剔除未引用资产。</summary>
    public bool StripUnusedAssets { get; set; } = true;

    /// <summary>是否包含调试信息。</summary>
    public bool IncludeDebugInfo { get; set; } = false;

    /// <summary>是否增量构建。</summary>
    public bool IncrementalBuild { get; set; } = true;

    /// <summary>热更新 URL（可选）。</summary>
    public string? RemoteUrl { get; set; }

    /// <summary>创建 Editor 配置。</summary>
    public static AddressableProfile CreateEditorProfile()
    {
        return new AddressableProfile
        {
            Name = "Editor",
            Compress = false,
            StripUnusedAssets = false,
            IncludeDebugInfo = true,
            IncrementalBuild = true
        };
    }

    /// <summary>创建 Development 配置。</summary>
    public static AddressableProfile CreateDevelopmentProfile()
    {
        return new AddressableProfile
        {
            Name = "Development",
            Compress = true,
            CompressionMethod = CompressionMethod.LZ4,
            CompressionLevel = 1,
            StripUnusedAssets = false,
            IncludeDebugInfo = true,
            IncrementalBuild = true
        };
    }

    /// <summary>创建 Release 配置。</summary>
    public static AddressableProfile CreateReleaseProfile()
    {
        return new AddressableProfile
        {
            Name = "Release",
            Compress = true,
            CompressionMethod = CompressionMethod.Zstd,
            CompressionLevel = 9,
            StripUnusedAssets = true,
            IncludeDebugInfo = false,
            IncrementalBuild = false
        };
    }
}

/// <summary>构建目标平台。</summary>
public enum BuildTarget
{
    Windows,
    Linux,
    MacOS,
    Android,
    iOS
}

/// <summary>压缩算法。</summary>
public enum CompressionMethod
{
    None,
    LZ4,
    Zstd
}
