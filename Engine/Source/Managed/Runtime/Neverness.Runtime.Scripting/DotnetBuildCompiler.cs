// ============================================================================
// DotnetBuildCompiler.cs - dotnet build 回退编译器
// ============================================================================
// 通过 Process.Start("dotnet build") 编译脚本。
// 作为 RoslynScriptCompiler 的回退方案，兼容 Source Generator。
//
// 使用场景：
// - ScriptCompilerMode.DotnetBuild 配置
// - Roslyn 编译失败时的 fallback
// - 需要 Source Generator 支持时
// ============================================================================

using System.Collections.Immutable;
using System.Diagnostics;
using Microsoft.CodeAnalysis;

namespace Neverness.Runtime.Scripting;

/// <summary>
/// dotnet build 回退编译器——通过 CLI 调用 MSBuild。
/// </summary>
public sealed class DotnetBuildCompiler : IScriptCompiler
{
    /// <inheritdoc />
    public ScriptCompilerMode Mode => ScriptCompilerMode.DotnetBuild;

    /// <inheritdoc />
    public ScriptCompilationResult Compile(ScriptCompilationContext context)
    {
        var stopwatch = Stopwatch.StartNew();

        try
        {
            // 需要 .csproj 文件来执行 dotnet build
            // 从 SourceFiles 推断项目目录
            var projectDir = InferProjectDirectory(context);
            var csprojPath = FindCsproj(projectDir);

            if (string.IsNullOrEmpty(csprojPath))
            {
                stopwatch.Stop();
                return new ScriptCompilationResult
                {
                    Success = false,
                    Diagnostics = ImmutableArray.Create(
                        Diagnostic.Create(
                            new DiagnosticDescriptor("NN0010", "NoCsproj", "No .csproj file found for dotnet build.", "Compilation", DiagnosticSeverity.Error, true),
                            Location.None)),
                    Duration = stopwatch.Elapsed
                };
            }

            // 执行 dotnet build
            var (success, output, errors) = RunDotnetBuild(csprojPath);

            stopwatch.Stop();

            if (!success)
            {
                return new ScriptCompilationResult
                {
                    Success = false,
                    Diagnostics = ImmutableArray.Create(
                        Diagnostic.Create(
                            new DiagnosticDescriptor("NN0011", "DotnetBuildFailed", string.Join("\n", errors), "Compilation", DiagnosticSeverity.Error, true),
                            Location.None)),
                    Duration = stopwatch.Elapsed
                };
            }

            // 读取输出 DLL
            var dllPath = FindOutputDll(projectDir, context.AssemblyName);
            var pdbPath = Path.ChangeExtension(dllPath, ".pdb");

            return new ScriptCompilationResult
            {
                Success = true,
                AssemblyBytes = File.Exists(dllPath) ? File.ReadAllBytes(dllPath) : null,
                PdbBytes = File.Exists(pdbPath) ? File.ReadAllBytes(pdbPath) : null,
                Diagnostics = ImmutableArray<Diagnostic>.Empty,
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
                        new DiagnosticDescriptor("NN0012", "DotnetBuildException", ex.Message, "Compilation", DiagnosticSeverity.Error, true),
                        Location.None)),
                Duration = stopwatch.Elapsed
            };
        }
    }

    // ========================================================================
    // 内部方法
    // ========================================================================

    private static (bool Success, string Output, List<string> Errors) RunDotnetBuild(string csprojPath)
    {
        var errors = new List<string>();

        try
        {
            var process = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = "dotnet",
                    Arguments = $"build \"{csprojPath}\" -c Debug --no-restore --nologo",
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    UseShellExecute = false,
                    CreateNoWindow = true
                }
            };

            process.Start();
            var output = process.StandardOutput.ReadToEnd();
            var error = process.StandardError.ReadToEnd();
            process.WaitForExit();

            if (process.ExitCode != 0)
            {
                if (!string.IsNullOrWhiteSpace(error)) errors.Add(error);
                if (!string.IsNullOrWhiteSpace(output)) errors.Add(output);
                return (false, output, errors);
            }

            return (true, output, errors);
        }
        catch (Exception ex)
        {
            errors.Add($"Failed to run dotnet build: {ex.Message}");
            return (false, "", errors);
        }
    }

    private static string? InferProjectDirectory(ScriptCompilationContext context)
    {
        if (context.SourceFiles.Count == 0)
            return null;

        // 从第一个源文件推断 Assets 目录，再上溯到项目根目录
        var firstFile = context.SourceFiles[0];
        var dir = Path.GetDirectoryName(firstFile);

        // 向上查找 .csproj
        while (!string.IsNullOrEmpty(dir))
        {
            if (Directory.GetFiles(dir, "*.csproj").Length > 0)
                return dir;
            dir = Path.GetDirectoryName(dir)!;
        }

        return null;
    }

    private static string? FindCsproj(string? projectDir)
    {
        if (string.IsNullOrEmpty(projectDir))
            return null;

        var csprojs = Directory.GetFiles(projectDir, "*.csproj");
        return csprojs.Length > 0 ? csprojs[0] : null;
    }

    private static string FindOutputDll(string? projectDir, string assemblyName)
    {
        if (string.IsNullOrEmpty(projectDir))
            return "";

        // OutputPath 在 .csproj 中设置为 Library/Scripts
        return Path.Combine(projectDir, "Library", "Scripts", $"{assemblyName}.dll");
    }
}
