// ============================================================================
// RoslynScriptCompiler.cs - Roslyn C# 脚本编译器
// ============================================================================
// 使用 Microsoft.CodeAnalysis.CSharp 进行 in-process 编译。
// 不调用 dotnet build / MSBuild / Process.Start。
//
// 流程：Parse → CSharpCompilation.Create → Emit(byte[])
// ============================================================================

using System.Collections.Immutable;
using System.Diagnostics;
using System.Text;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.Text;
using Microsoft.CodeAnalysis.Emit;

namespace Neverness.Runtime.Scripting;

/// <summary>
/// Roslyn C# 脚本编译器——in-process 编译，无进程启动开销。
/// </summary>
public sealed class RoslynScriptCompiler : IScriptCompiler
{
    /// <inheritdoc />
    public ScriptCompilerMode Mode => ScriptCompilerMode.Roslyn;

    /// <inheritdoc />
    public ScriptCompilationResult Compile(ScriptCompilationContext context)
    {
        var stopwatch = Stopwatch.StartNew();

        try
        {
            // 1. Parse 源文件 → SyntaxTree
            var syntaxTrees = new List<SyntaxTree>();
            foreach (var sourceFile in context.SourceFiles)
            {
                if (!File.Exists(sourceFile))
                    continue;

                var sourceText = SourceText.From(File.ReadAllText(sourceFile, Encoding.UTF8), Encoding.UTF8);
                var tree = CSharpSyntaxTree.ParseText(
                    sourceText,
                    path: sourceFile);
                syntaxTrees.Add(tree);
            }

            if (syntaxTrees.Count == 0)
            {
                stopwatch.Stop();
                return new ScriptCompilationResult
                {
                    Success = false,
                    Diagnostics = ImmutableArray.Create(
                        Diagnostic.Create(
                            new DiagnosticDescriptor("NN0001", "NoSourceFiles", "No source files found.", "Compilation", DiagnosticSeverity.Error, true),
                            Location.None)),
                    Duration = stopwatch.Elapsed
                };
            }

            // 2. 创建 Compilation
            var compilationOptions = new CSharpCompilationOptions(
                OutputKind.DynamicallyLinkedLibrary,
                optimizationLevel: context.Optimize ? OptimizationLevel.Release : OptimizationLevel.Debug,
                generalDiagnosticOption: ReportDiagnostic.Default,
                nullableContextOptions: NullableContextOptions.Enable);

            var parseOptions = new CSharpParseOptions(
                LanguageVersion.Latest,
                DocumentationMode.None,
                SourceCodeKind.Regular);

            var compilation = CSharpCompilation.Create(
                context.AssemblyName,
                syntaxTrees,
                context.References,
                compilationOptions);

            // 3. Emit → byte[]
            using var dllStream = new MemoryStream();
            using var pdbStream = context.GeneratePdb ? new MemoryStream() : null;

            var emitOptions = new EmitOptions(
                debugInformationFormat: DebugInformationFormat.PortablePdb);

            var emitResult = compilation.Emit(
                dllStream,
                pdbStream,
                options: emitOptions);

            stopwatch.Stop();

            if (!emitResult.Success)
            {
                return new ScriptCompilationResult
                {
                    Success = false,
                    Diagnostics = emitResult.Diagnostics.ToImmutableArray(),
                    Duration = stopwatch.Elapsed
                };
            }

            return new ScriptCompilationResult
            {
                Success = true,
                AssemblyBytes = dllStream.ToArray(),
                PdbBytes = pdbStream?.ToArray(),
                Diagnostics = emitResult.Diagnostics.ToImmutableArray(),
                Duration = stopwatch.Elapsed
            };
        }
        catch (Exception ex)
        {
            stopwatch.Stop();
            return new ScriptCompilationResult
            {
                Success = false,
                Diagnostics = ImmutableArray.Create(
                    Diagnostic.Create(
                        new DiagnosticDescriptor("NN0002", "CompilationException", ex.Message, "Compilation", DiagnosticSeverity.Error, true),
                        Location.None)),
                Duration = stopwatch.Elapsed
            };
        }
    }
}
