// ============================================================================
// ScriptCompiler.cs - 脚本编译器
// ============================================================================
// 执行 dotnet build 编译用户脚本。
// 工程文件由 ScriptProjectGenerator 生成，本类只负责编译。
//
// 优化：
// - OutputPath 写在 .csproj 中（不用 -o 标志），MSBuild 增量编译正常工作
// - --no-restore：跳过 NuGet restore（首次需调用 Restore()）
// - --nologo：减少输出噪音
//
// ⚠️ 仅在 Editor JIT 模式下使用。
// Release 模式使用 Source Generator 静态注册，不使用此编译器。
// ============================================================================

using System.Diagnostics;

namespace Neverness.Gameplay;

/// <summary>
/// 脚本编译器——执行 dotnet build。
/// </summary>
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
    /// 执行 dotnet restore（首次编译前调用，之后不需要）。
    /// </summary>
    /// <param name="csprojPath">.csproj 文件路径。</param>
    /// <returns>是否成功。</returns>
    public bool Restore(string csprojPath)
    {
        if (!File.Exists(csprojPath))
            return false;

        var (success, _, errors) = RunDotnet("restore", csprojPath, null);
        if (!success)
        {
            Console.WriteLine($"[ScriptCompiler] dotnet restore failed: {string.Join("; ", errors)}");
        }
        return success;
    }

    /// <summary>
    /// 编译用户脚本。
    /// </summary>
    /// <param name="csprojPath">.csproj 文件路径。</param>
    /// <param name="outputPath">输出目录（与 .csproj 中 OutputPath 一致）。</param>
    /// <param name="assemblyName">输出程序集名称。</param>
    /// <param name="configuration">编译配置（Debug/Release）。</param>
    /// <returns>编译结果。</returns>
    public CompileResult Compile(string csprojPath, string outputPath, string assemblyName = "Assembly-CSharp", string configuration = "Debug")
    {
        var stopwatch = Stopwatch.StartNew();

        try
        {
            if (!File.Exists(csprojPath))
            {
                return new CompileResult
                {
                    Success = false,
                    Errors = new[] { $"Project file not found: {csprojPath}" },
                    ElapsedMilliseconds = stopwatch.ElapsedMilliseconds
                };
            }

            // 执行 dotnet build --no-restore --nologo
            // --no-restore: 跳过 NuGet restore（需先调用 Restore()）
            // --nologo: 减少输出噪音
            // 不用 -o: OutputPath 已写在 .csproj 中，MSBuild 增量编译正常工作
            var (success, output, errors) = RunDotnet("build", csprojPath, configuration);

            stopwatch.Stop();

            return new CompileResult
            {
                Success = success,
                OutputPath = success ? Path.Combine(outputPath, $"{assemblyName}.dll") : null,
                Errors = errors,
                Warnings = ParseWarnings(output),
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

    /// <summary>执行 dotnet 命令。</summary>
    private static (bool Success, string Output, List<string> Errors) RunDotnet(
        string command, string csprojPath, string? configuration)
    {
        var errors = new List<string>();

        try
        {
            var args = configuration != null
                ? $"{command} \"{csprojPath}\" -c {configuration} --no-restore --nologo"
                : $"{command} \"{csprojPath}\"";

            var process = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = "dotnet",
                    Arguments = args,
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
                if (!string.IsNullOrWhiteSpace(error))
                    errors.Add(error);
                if (!string.IsNullOrWhiteSpace(output))
                    errors.Add(output);
                return (false, output, errors);
            }

            return (true, output, errors);
        }
        catch (Exception ex)
        {
            errors.Add($"Failed to run dotnet {command}: {ex.Message}");
            return (false, "", errors);
        }
    }

    /// <summary>从输出中解析警告。</summary>
    private static List<string> ParseWarnings(string output)
    {
        var warnings = new List<string>();
        foreach (var line in output.Split('\n'))
        {
            if (line.Contains("warning", StringComparison.OrdinalIgnoreCase))
            {
                warnings.Add(line.Trim());
            }
        }
        return warnings;
    }
}
