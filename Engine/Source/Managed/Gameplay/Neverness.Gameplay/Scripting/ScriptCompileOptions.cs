// ============================================================================
// ScriptCompileOptions.cs - 脚本编译选项
// ============================================================================
// 脚本编译的配置选项。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 脚本编译选项。
/// </summary>
public sealed class ScriptCompileOptions
{
    // ========================================================================
    // 属性
    // ========================================================================

    /// <summary>脚本根目录（包含 .cs 文件的目录）。</summary>
    public string ScriptRoot { get; set; } = string.Empty;

    /// <summary>输出目录（编译产物输出位置）。</summary>
    public string OutputPath { get; set; } = string.Empty;

    /// <summary>目标框架。</summary>
    public string TargetFramework { get; set; } = "net10.0";

    /// <summary>编译配置（Debug/Release）。</summary>
    public string Configuration { get; set; } = "Debug";

    /// <summary>是否优化。</summary>
    public bool Optimize { get; set; }

    /// <summary>是否启用 Roslyn 脚本编译。</summary>
    public bool EnableRoslyn { get; set; } = true;

    /// <summary>额外引用程序集路径。</summary>
    public IReadOnlyList<string> AdditionalReferences { get; set; } = Array.Empty<string>();

    /// <summary>额外源文件路径。</summary>
    public IReadOnlyList<string> AdditionalSources { get; set; } = Array.Empty<string>();

    /// <summary>排除的文件模式。</summary>
    public IReadOnlyList<string> ExcludePatterns { get; set; } = Array.Empty<string>();

    /// <summary>包含的目录（除了 ScriptRoot）。</summary>
    public IReadOnlyList<string> IncludeDirectories { get; set; } = Array.Empty<string>();

    // ========================================================================
    // 默认值
    // ========================================================================

    /// <summary>创建默认编译选项。</summary>
    /// <param name="scriptRoot">脚本根目录。</param>
    /// <param name="outputPath">输出目录。</param>
    /// <returns>编译选项。</returns>
    public static ScriptCompileOptions CreateDefault(string scriptRoot, string outputPath)
    {
        return new ScriptCompileOptions
        {
            ScriptRoot = scriptRoot,
            OutputPath = outputPath,
            TargetFramework = "net10.0",
            Configuration = "Debug",
            Optimize = false,
            EnableRoslyn = true
        };
    }
}
