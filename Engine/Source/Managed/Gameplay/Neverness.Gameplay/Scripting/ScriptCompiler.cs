// ============================================================================
// ScriptCompiler.cs - 脚本编译器
// ============================================================================
// 编译用户脚本，支持增量编译和热重载。
// 仅在 Editor JIT 模式下使用。
// ============================================================================

using System.Diagnostics;
using System.Text;

namespace Neverness.Gameplay;

/// <summary>
/// 脚本编译器：编译用户脚本。
/// </summary>
/// <remarks>
/// ⚠️ 仅在 Editor JIT 模式下使用。
/// Release 模式使用 Source Generator 静态注册，不使用此编译器。
/// </remarks>
public sealed class ScriptCompiler
{
    // ========================================================================
    // 内部类型
    // ========================================================================

    /// <summary>编译结果。</summary>
    public sealed class CompileResult
    {
        /// <summary>是否成功。</summary>
        public bool Success { get; init; }

        /// <summary>输出程序集路径。</summary>
        public string? OutputPath { get; init; }

        /// <summary>错误信息。</summary>
        public IReadOnlyList<string> Errors { get; init; } = Array.Empty<string>();

        /// <summary>警告信息。</summary>
        public IReadOnlyList<string> Warnings { get; init; } = Array.Empty<string>();

        /// <summary>编译耗时（毫秒）。</summary>
        public long ElapsedMilliseconds { get; init; }
    }

    // ========================================================================
    // 公共方法
    // ========================================================================

    /// <summary>
    /// 编译用户脚本。
    /// </summary>
    /// <param name="options">编译选项。</param>
    /// <returns>编译结果。</returns>
    public CompileResult Compile(ScriptCompileOptions options)
    {
        var stopwatch = Stopwatch.StartNew();

        try
        {
            // 1. 扫描源文件
            var sourceFiles = ScanSourceFiles(options);
            if (sourceFiles.Count == 0)
            {
                return new CompileResult
                {
                    Success = false,
                    Errors = new[] { "No source files found." }
                };
            }

            // 2. 生成临时 csproj
            var csprojPath = GenerateCsproj(options, sourceFiles);

            // 3. 执行 dotnet build
            var result = RunDotnetBuild(csprojPath, options);

            stopwatch.Stop();

            return new CompileResult
            {
                Success = result.Success,
                OutputPath = result.Success ? Path.Combine(options.OutputPath, "UserScripts.dll") : null,
                Errors = result.Errors,
                Warnings = result.Warnings,
                ElapsedMilliseconds = stopwatch.ElapsedMilliseconds
            };
        }
        catch (Exception ex)
        {
            stopwatch.Stop();
            return new CompileResult
            {
                Success = false,
                Errors = new[] { ex.Message },
                ElapsedMilliseconds = stopwatch.ElapsedMilliseconds
            };
        }
    }

    // ========================================================================
    // 内部方法
    // ========================================================================

    /// <summary>扫描源文件。</summary>
    private List<string> ScanSourceFiles(ScriptCompileOptions options)
    {
        var sourceFiles = new List<string>();
        var scriptRoot = Path.GetFullPath(options.ScriptRoot);

        if (!Directory.Exists(scriptRoot))
        {
            return sourceFiles;
        }

        // 扫描主目录
        sourceFiles.AddRange(Directory.GetFiles(scriptRoot, "*.cs", SearchOption.AllDirectories));

        // 扫描额外目录
        foreach (var dir in options.IncludeDirectories)
        {
            var fullPath = Path.GetFullPath(dir);
            if (Directory.Exists(fullPath))
            {
                sourceFiles.AddRange(Directory.GetFiles(fullPath, "*.cs", SearchOption.AllDirectories));
            }
        }

        // 添加额外源文件
        sourceFiles.AddRange(options.AdditionalSources);

        // 过滤排除的文件
        var filteredFiles = new List<string>();
        foreach (var file in sourceFiles)
        {
            var relativePath = Path.GetRelativePath(scriptRoot, file);
            var excluded = false;

            foreach (var pattern in options.ExcludePatterns)
            {
                if (relativePath.StartsWith(pattern, StringComparison.OrdinalIgnoreCase))
                {
                    excluded = true;
                    break;
                }
            }

            if (!excluded)
            {
                filteredFiles.Add(file);
            }
        }

        return filteredFiles;
    }

    /// <summary>生成临时 csproj。</summary>
    private string GenerateCsproj(ScriptCompileOptions options, List<string> sourceFiles)
    {
        var tempDir = Path.Combine(Path.GetTempPath(), "Neverness.ScriptCompile", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(tempDir);

        var csprojPath = Path.Combine(tempDir, "UserScripts.csproj");
        var sb = new StringBuilder();

        sb.AppendLine("<Project Sdk=\"Microsoft.NET.Sdk\">");
        sb.AppendLine("  <PropertyGroup>");
        sb.AppendLine($"    <TargetFramework>{options.TargetFramework}</TargetFramework>");
        sb.AppendLine("    <AssemblyName>UserScripts</AssemblyName>");
        sb.AppendLine("    <OutputType>Library</OutputType>");
        sb.AppendLine("    <Nullable>enable</Nullable>");
        sb.AppendLine("    <ImplicitUsings>enable</ImplicitUsings>");

        if (options.Optimize)
        {
            sb.AppendLine("    <Optimize>true</Optimize>");
        }

        sb.AppendLine("  </PropertyGroup>");

        // 引用 Gameplay 程序集
        sb.AppendLine("  <ItemGroup>");
        sb.AppendLine("    <ProjectReference Include=\"Neverness.Gameplay.csproj\" />");
        sb.AppendLine("  </ItemGroup>");

        // 额外引用
        if (options.AdditionalReferences.Count > 0)
        {
            sb.AppendLine("  <ItemGroup>");
            foreach (var reference in options.AdditionalReferences)
            {
                sb.AppendLine($"    <Reference Include=\"{Path.GetFileNameWithoutExtension(reference)}\" />");
            }
            sb.AppendLine("  </ItemGroup>");
        }

        sb.AppendLine("</Project>");

        File.WriteAllText(csprojPath, sb.ToString());

        return csprojPath;
    }

    /// <summary>执行 dotnet build。</summary>
    private (bool Success, List<string> Errors, List<string> Warnings) RunDotnetBuild(
        string csprojPath,
        ScriptCompileOptions options)
    {
        var errors = new List<string>();
        var warnings = new List<string>();

        try
        {
            var process = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = "dotnet",
                    Arguments = $"build \"{csprojPath}\" -c {options.Configuration} -o \"{options.OutputPath}\"",
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
                errors.Add(error);
                return (false, errors, warnings);
            }

            // 解析警告
            foreach (var line in output.Split('\n'))
            {
                if (line.Contains("warning", StringComparison.OrdinalIgnoreCase))
                {
                    warnings.Add(line.Trim());
                }
            }

            return (true, errors, warnings);
        }
        catch (Exception ex)
        {
            errors.Add($"Failed to run dotnet build: {ex.Message}");
            return (false, errors, warnings);
        }
    }
}
