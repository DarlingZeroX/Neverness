using System.Runtime.InteropServices;
using System.Text;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产编译器 C# 包装。
///
/// 调用 C++ NNAssetCooker 进行 .nnpack 构建。
/// </summary>
public static unsafe class AssetCooker
{
    /// <summary>
    /// 执行构建。
    /// </summary>
    public static CookResult Cook(BuildProfile profile)
    {
        var api = GetCookerApi();
        if (api == null)
            return CookResult.Fail("AssetCooker API 未接线");

        /* 创建清单 */
        var manifestHandle = api.CreateManifest();
        if (manifestHandle == 0)
            return CookResult.Fail("无法创建构建清单");

        try
        {
            /* 设置路径 */
            var outputRootBytes = Encoding.UTF8.GetBytes(profile.OutputRoot.FullPath);
            fixed (byte* p = outputRootBytes)
                api.SetOutputRoot(manifestHandle, p);

            var libraryRootBytes = Encoding.UTF8.GetBytes(profile.LibraryRoot.FullPath);
            fixed (byte* p = libraryRootBytes)
                api.SetLibraryRoot(manifestHandle, p);

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

                uint groupIndex = 0;
                if (profile.GroupManager != null)
                {
                    /* TODO: 查找资产所属分组 */
                }

                var sourcePathBytes = Encoding.UTF8.GetBytes(sourcePath);
                fixed (byte* p = sourcePathBytes)
                    api.AddAsset(manifestHandle, guid.ToNative(), typeId, p, groupIndex);
            }

            /* 添加分组 */
            if (profile.GroupManager != null)
            {
                foreach (var group in profile.GroupManager.Groups)
                {
                    var outputPath = profile.OutputRoot.Combine(group.Name + ".nnpack").FullPath;
                    var nameBytes = Encoding.UTF8.GetBytes(group.Name);
                    var addrBytes = Encoding.UTF8.GetBytes(group.Address);
                    var pathBytes = Encoding.UTF8.GetBytes(outputPath);
                    fixed (byte* pName = nameBytes)
                    fixed (byte* pAddr = addrBytes)
                    fixed (byte* pPath = pathBytes)
                        api.AddGroup(manifestHandle, pName, pAddr, (uint)group.Strategy, pPath);
                }
            }

            /* 执行构建 */
            var nativeResult = api.Cook(manifestHandle);

            return new CookResult
            {
                Success = nativeResult.Success != 0,
                TotalAssets = nativeResult.TotalAssets,
                CookedAssets = nativeResult.CookedAssets,
                FailedAssets = nativeResult.FailedAssets,
                GeneratedPacks = nativeResult.GeneratedPacks,
                ElapsedSeconds = nativeResult.ElapsedSeconds
            };
        }
        finally
        {
            api.DestroyManifest(manifestHandle);
        }
    }

    private static CookerApiTable? s_api;

    private static CookerApiTable? GetCookerApi()
    {
        if (s_api != null) return s_api;

        /* TODO: 从 NativeApiProvider 获取完整 API 表 */
        /* 当前创建空表 */
        s_api = new CookerApiTable();
        return s_api;
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

/// <summary>Cooker API 函数表（C# 镜像）。</summary>
public sealed unsafe class CookerApiTable
{
    public delegate* unmanaged[Stdcall]<ulong> CreateManifest { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, void> DestroyManifest { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, byte*, void> SetOutputRoot { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, byte*, void> SetLibraryRoot { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, NNGuid, ulong, byte*, uint, void> AddAsset { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, byte*, byte*, uint, byte*, void> AddGroup { get; init; }
    public delegate* unmanaged[Stdcall]<ulong, NativeCookResultData> Cook { get; init; }
}

[StructLayout(LayoutKind.Sequential)]
public struct NativeCookResultData
{
    public int Success;
    public uint TotalAssets;
    public uint CookedAssets;
    public uint FailedAssets;
    public uint GeneratedPacks;
    public double ElapsedSeconds;
}
