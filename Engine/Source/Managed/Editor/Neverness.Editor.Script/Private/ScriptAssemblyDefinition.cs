// ============================================================================
// ScriptAssemblyDefinition.cs - 脚本程序集定义
// ============================================================================
// 描述一个可编译的脚本程序集。
// 当前只有 Assembly-CSharp，未来支持 asmdef 多程序集。
// ============================================================================

namespace Neverness.Editor.Script.Private;

/// <summary>
/// 脚本程序集定义——描述一个可编译的脚本程序集。
/// </summary>
public sealed class ScriptAssemblyDefinition
{
    /// <summary>程序集名称（如 Assembly-CSharp）。</summary>
    public required string Name { get; init; }

    /// <summary>源文件 glob 模式列表（如 ["Assets/**/*.cs"]）。</summary>
    public required IReadOnlyList<string> SourceGlobs { get; init; }

    /// <summary>引用的引擎程序集名称列表（如 ["Neverness.Gameplay"]）。</summary>
    public required IReadOnlyList<string> References { get; init; }

    /// <summary>输出目录。</summary>
    public required string OutputPath { get; init; }

    /// <summary>目标框架（从宿主运行时自动读取）。</summary>
    public string TargetFramework { get; init; } = DetectTargetFramework();

    /// <summary>编译配置（Debug/Release）。</summary>
    public string Configuration { get; init; } = "Debug";

    /// <summary>
    /// 检测宿主运行时的目标框架。
    /// 从 EntityBehaviour 所在程序集读取 TargetFrameworkAttribute，
    /// 确保用户脚本与 Editor 运行时一致。
    /// </summary>
    private static string DetectTargetFramework()
    {
        var attr = typeof(Gameplay.EntityBehaviour).Assembly
            .GetCustomAttributes(typeof(System.Runtime.Versioning.TargetFrameworkAttribute), false)
            .FirstOrDefault() as System.Runtime.Versioning.TargetFrameworkAttribute;

        if (attr != null)
        {
            // TargetFrameworkAttribute 格式: ".NETCoreApp,Version=v10.0"
            // 提取 "net10.0"
            var version = attr.FrameworkName;
            var match = System.Text.RegularExpressions.Regex.Match(version, @"Version=v(\d+\.\d+)");
            if (match.Success)
            {
                return $"net{match.Groups[1].Value}";
            }
        }

        // 回退
        return "net10.0";
    }
}
