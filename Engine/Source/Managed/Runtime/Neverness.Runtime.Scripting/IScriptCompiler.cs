// ============================================================================
// IScriptCompiler.cs - 脚本编译器接口
// ============================================================================
// 统一编译抽象，支持 Roslyn 和 DotnetBuild 两种实现。
// ============================================================================

using System.Collections.Immutable;
using Microsoft.CodeAnalysis;

namespace Neverness.Runtime.Scripting;

/// <summary>编译器模式。</summary>
public enum ScriptCompilerMode
{
    /// <summary>Roslyn in-process 编译（默认，快速）。</summary>
    Roslyn,

    /// <summary>dotnet build 回退方案（兼容 Source Generator）。</summary>
    DotnetBuild
}

/// <summary>脚本编译上下文。</summary>
public sealed class ScriptCompilationContext
{
    /// <summary>输出程序集名称（如 Assembly-CSharp）。</summary>
    public required string AssemblyName { get; init; }

    /// <summary>源文件路径列表。</summary>
    public required IReadOnlyList<string> SourceFiles { get; init; }

    /// <summary>MetadataReference 列表（BCL + 引擎）。</summary>
    public required ImmutableArray<MetadataReference> References { get; init; }

    /// <summary>是否生成 PDB（调试符号）。</summary>
    public bool GeneratePdb { get; init; } = true;

    /// <summary>是否优化。</summary>
    public bool Optimize { get; init; } = false;

    /// <summary>目标框架（如 net10.0）。</summary>
    public string TargetFramework { get; init; } = "net10.0";
}

/// <summary>脚本编译结果。</summary>
public sealed class ScriptCompilationResult
{
    /// <summary>是否成功。</summary>
    public required bool Success { get; init; }

    /// <summary>编译产物（成功时非 null）。</summary>
    public byte[]? AssemblyBytes { get; init; }

    /// <summary>PDB 调试符号（成功时非 null，GeneratePdb=true 时生成）。</summary>
    public byte[]? PdbBytes { get; init; }

    /// <summary>编译诊断信息。</summary>
    public ImmutableArray<Diagnostic> Diagnostics { get; init; } = ImmutableArray<Diagnostic>.Empty;

    /// <summary>编译耗时。</summary>
    public TimeSpan Duration { get; init; }

    /// <summary>错误数量。</summary>
    public int ErrorCount => Diagnostics.Count(d => d.Severity == DiagnosticSeverity.Error);

    /// <summary>警告数量。</summary>
    public int WarningCount => Diagnostics.Count(d => d.Severity == DiagnosticSeverity.Warning);
}

/// <summary>
/// 脚本编译器接口。
/// </summary>
public interface IScriptCompiler
{
    /// <summary>编译器模式。</summary>
    ScriptCompilerMode Mode { get; }

    /// <summary>编译脚本。</summary>
    /// <param name="context">编译上下文。</param>
    /// <returns>编译结果。</returns>
    ScriptCompilationResult Compile(ScriptCompilationContext context);
}
