// ============================================================================
// IReferenceAssemblyProvider.cs - 引用程序集提供器接口
// ============================================================================
// 为 Roslyn 编译器提供 MetadataReference。
// Phase 1: TpaReferenceProvider（TPA 路径）
// Phase 2: RefPackReferenceProvider（Microsoft.NETCore.App.Ref）
// ============================================================================

using System.Collections.Immutable;
using Microsoft.CodeAnalysis;

namespace Neverness.Runtime.Scripting;

/// <summary>
/// 引用程序集提供器——为 Roslyn 编译器提供 MetadataReference。
/// </summary>
public interface IReferenceAssemblyProvider
{
    /// <summary>获取所有引用程序集的 MetadataReference 列表。</summary>
    ImmutableArray<MetadataReference> GetReferences();
}
