using System.Diagnostics;
using System.Text;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产编译器（纯 C# 实现）。
///
/// 读取 .nnasset 文件，按分组打包为 .nnpack。
/// 不依赖 C++ NNAssetCooker。
/// </summary>
public static class AssetCooker
{
    /// <summary>
    /// 执行构建。
    /// </summary>
    public static CookResult Cook(BuildProfile profile)
    {
        var sw = Stopwatch.StartNew();
        var manifest = new CookManifest
        {
            OutputRoot = profile.OutputRoot.FullPath,
            LibraryRoot = profile.LibraryRoot.FullPath
        };

        /* 收集资产 */
        var allAssets = EditorAssetDatabase.AllAssets;
        for (int i = 0; i < allAssets.Count; i++)
        {
            var guid = allAssets[i];
            var typeId = EditorAssetDatabase.GetTypeId(guid);

            string sourcePath = string.Empty;
            if (EditorAssetDatabase.TryGetPath(guid, out var vp) && !vp.IsEmpty)
            {
                if (EditorAssetDatabase.TryGetSourcePath(vp, out var sp) && !sp.IsEmpty)
                    sourcePath = sp.FullPath;
                else
                    sourcePath = vp.FullPath;
            }

            /* 查找资产所属分组 */
            uint groupIndex = 0;
            if (profile.GroupManager != null)
            {
                var groups = profile.GroupManager.FindGroupsContaining(guid);
                if (groups.Count > 0)
                {
                    var groupName = groups[0].Name;
                    for (int g = 0; g < profile.GroupManager.Groups.Count; g++)
                    {
                        if (profile.GroupManager.Groups[g].Name == groupName)
                        {
                            groupIndex = (uint)g;
                            break;
                        }
                    }
                }
            }

            manifest.AddAsset(new CookAssetEntry
            {
                Guid = guid,
                TypeId = typeId,
                SourcePath = sourcePath,
                GroupIndex = groupIndex,
                IncludeInBuild = true
            });
        }

        /* 添加分组 */
        if (profile.GroupManager != null)
        {
            for (int i = 0; i < profile.GroupManager.Groups.Count; i++)
            {
                var group = profile.GroupManager.Groups[i];
                var outputPath = profile.OutputRoot.Combine(group.Name + ".nnpack").FullPath;
                manifest.AddGroup(new CookGroup
                {
                    Name = group.Name,
                    Address = group.Address,
                    Strategy = (uint)group.Strategy,
                    OutputPath = outputPath
                });
            }
        }

        /* 执行构建 */
        return CookManifest(manifest, sw);
    }

    /// <summary>
    /// 根据清单执行构建。
    /// </summary>
    private static CookResult CookManifest(CookManifest manifest, Stopwatch sw)
    {
        uint cookedAssets = 0;
        uint failedAssets = 0;
        uint generatedPacks = 0;

        if (manifest.GroupCount == 0)
        {
            /* 无分组：所有资产打包到 default.nnpack */
            var outputPath = Path.Combine(manifest.OutputRoot, "default.nnpack");
            var assets = manifest.Assets
                .Where(a => a.IncludeInBuild)
                .Select(a => ReadAssetData(a, manifest.LibraryRoot))
                .Where(a => a.HasValue)
                .Select(a => a!.Value)
                .ToList();

            if (PackageBuilder.Build(assets, outputPath, "default"))
            {
                cookedAssets = (uint)assets.Count;
                generatedPacks = 1;
            }
            else
            {
                failedAssets = (uint)assets.Count;
            }
        }
        else
        {
            /* 按分组构建 */
            for (int i = 0; i < manifest.GroupCount; i++)
            {
                var group = manifest.Groups[i];
                var groupAssets = manifest.Assets
                    .Where(a => a.GroupIndex == i && a.IncludeInBuild)
                    .ToList();

                if (groupAssets.Count == 0)
                    continue;

                var outputPath = !string.IsNullOrEmpty(group.OutputPath)
                    ? group.OutputPath
                    : Path.Combine(manifest.OutputRoot, group.Name + ".nnpack");

                var assets = groupAssets
                    .Select(a => ReadAssetData(a, manifest.LibraryRoot))
                    .Where(a => a.HasValue)
                    .Select(a => a!.Value)
                    .ToList();

                if (assets.Count == 0)
                {
                    failedAssets += (uint)groupAssets.Count;
                    continue;
                }

                if (PackageBuilder.Build(assets, outputPath, group.Name))
                {
                    cookedAssets += (uint)assets.Count;
                    generatedPacks++;
                }
                else
                {
                    failedAssets += (uint)assets.Count;
                }
            }
        }

        sw.Stop();

        return new CookResult
        {
            Success = failedAssets == 0,
            TotalAssets = (uint)manifest.AssetCount,
            CookedAssets = cookedAssets,
            FailedAssets = failedAssets,
            GeneratedPacks = generatedPacks,
            ElapsedSeconds = sw.Elapsed.TotalSeconds
        };
    }

    /// <summary>
    /// 读取单个资产的二进制数据。
    /// </summary>
    private static (GUID guid, ulong typeId, byte[] data)? ReadAssetData(
        CookAssetEntry entry, string libraryRoot)
    {
        try
        {
            var sourcePath = entry.SourcePath;
            if (!Path.IsPathRooted(sourcePath))
                sourcePath = Path.Combine(libraryRoot, sourcePath);

            if (!File.Exists(sourcePath))
                return null;

            var data = File.ReadAllBytes(sourcePath);
            if (data.Length < 32)
                return null;

            return (entry.Guid, entry.TypeId, data);
        }
        catch
        {
            return null;
        }
    }
}

/// <summary>构建结果。</summary>
public sealed class CookResult
{
    public bool Success { get; init; }
    public uint TotalAssets { get; init; }
    public uint CookedAssets { get; init; }
    public uint FailedAssets { get; init; }
    public uint GeneratedPacks { get; init; }
    public double ElapsedSeconds { get; init; }

    public static CookResult Fail(string error) => new() { Success = false };

    public override string ToString()
    {
        return Success
            ? $"构建成功: {CookedAssets}/{TotalAssets} 资产, {GeneratedPacks} 个包, 耗时 {ElapsedSeconds:F1}s"
            : $"构建失败: {CookedAssets}/{TotalAssets} 资产";
    }
}
