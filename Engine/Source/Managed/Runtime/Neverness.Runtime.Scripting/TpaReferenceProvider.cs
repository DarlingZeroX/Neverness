// ============================================================================
// TpaReferenceProvider.cs - TPA（Trusted Platform Assemblies）引用提供器
// ============================================================================
// Phase 1 实现：从 AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES") 获取 BCL 路径。
// 全平台通用（Windows/Linux/Mac），不需要查找 SDK 安装路径。
//
// Phase 2 计划：替换为 RefPackReferenceProvider（Microsoft.NETCore.App.Ref）。
// ============================================================================

using System.Collections.Immutable;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;

namespace Neverness.Runtime.Scripting;

/// <summary>
/// TPA 引用提供器——从运行时 TPA 列表获取 BCL 程序集路径。
/// </summary>
public sealed class TpaReferenceProvider : IReferenceAssemblyProvider
{
    private ImmutableArray<MetadataReference>? _cache;

    /// <inheritdoc />
    public ImmutableArray<MetadataReference> GetReferences()
    {
        if (_cache.HasValue)
            return _cache.Value;

        var builder = ImmutableArray.CreateBuilder<MetadataReference>();

        var tpa = AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES") as string;
        if (!string.IsNullOrEmpty(tpa))
        {
            foreach (var path in tpa.Split(Path.PathSeparator))
            {
                if (File.Exists(path))
                {
                    try
                    {
                        builder.Add(MetadataReference.CreateFromFile(path));
                    }
                    catch
                    {
                        // 跳过无法加载的程序集
                    }
                }
            }
        }

        _cache = builder.ToImmutable();
        return _cache.Value;
    }
}
