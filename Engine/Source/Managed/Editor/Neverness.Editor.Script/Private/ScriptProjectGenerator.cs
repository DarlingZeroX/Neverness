// ============================================================================
// ScriptProjectGenerator.cs - 脚本工程文件生成器
// ============================================================================
// 生成 Game.sln + Assembly-CSharp.csproj（持久化，非临时）。
//
// 设计原则：
// - WriteIfChanged：内容不变不写入，避免 IDE 反复 Reload
// - 确定性 GUID：基于 assembly name 的 MD5，避免 VS 每次重新加载
// - 相对路径 HintPath：换机器/CI/Git Clone 不失效
// - glob 源文件：增删 .cs 不需要重新生成 csproj
// ============================================================================

using System.Security.Cryptography;
using System.Text;

namespace Neverness.Editor.Script.Private;

/// <summary>
/// 脚本工程文件生成器——生成 Game.sln 和 Assembly-CSharp.csproj。
/// </summary>
public sealed class ScriptProjectGenerator
{
    private readonly IEngineAssemblyResolver _engineResolver;

    public ScriptProjectGenerator(IEngineAssemblyResolver engineResolver)
    {
        _engineResolver = engineResolver;
    }

    /// <summary>
    /// 生成或更新 .sln + .csproj。
    /// </summary>
    /// <param name="projectRoot">项目根目录（OS 绝对路径）。</param>
    /// <param name="assemblies">要生成的程序集定义列表。</param>
    public void GenerateProjectFiles(string projectRoot, IReadOnlyList<ScriptAssemblyDefinition> assemblies)
    {
        var slnPath = Path.Combine(projectRoot, "Game.sln");

        // 生成每个 .csproj
        var csprojFileNames = new List<string>();
        foreach (var assembly in assemblies)
        {
            var csprojPath = Path.Combine(projectRoot, $"{assembly.Name}.csproj");
            var csprojContent = GenerateCsprojContent(assembly, projectRoot);
            WriteIfChanged(csprojPath, csprojContent);
            csprojFileNames.Add($"{assembly.Name}.csproj");
            Console.WriteLine($"[ScriptProjectGenerator] Generated: {assembly.Name}.csproj");
        }

        // 生成 .sln
        var slnContent = GenerateSlnContent(assemblies, csprojFileNames);
        WriteIfChanged(slnPath, slnContent);
        Console.WriteLine($"[ScriptProjectGenerator] Generated: Game.sln");
    }

    /// <summary>生成 .csproj 内容。</summary>
    private string GenerateCsprojContent(ScriptAssemblyDefinition assembly, string projectRoot)
    {
        var sb = new StringBuilder();

        sb.AppendLine("<Project Sdk=\"Microsoft.NET.Sdk\">");
        sb.AppendLine("  <PropertyGroup>");
        sb.AppendLine($"    <TargetFramework>{assembly.TargetFramework}</TargetFramework>");
        sb.AppendLine($"    <AssemblyName>{assembly.Name}</AssemblyName>");
        sb.AppendLine("    <OutputType>Library</OutputType>");
        sb.AppendLine("    <Nullable>enable</Nullable>");
        sb.AppendLine("    <ImplicitUsings>enable</ImplicitUsings>");
        sb.AppendLine("    <EnableDefaultCompileItems>false</EnableDefaultCompileItems>");
        sb.AppendLine($"    <OutputPath>{assembly.OutputPath}</OutputPath>");
        sb.AppendLine("    <AppendTargetFrameworkToOutputPath>false</AppendTargetFrameworkToOutputPath>");
        // ── 减少 MSBuild 开销 ──
        sb.AppendLine("    <EnableSourceLink>false</EnableSourceLink>");
        sb.AppendLine("    <EnableSourceControlManagerQueries>false</EnableSourceControlManagerQueries>");
        sb.AppendLine("    <GenerateAssemblyInfo>false</GenerateAssemblyInfo>");
        sb.AppendLine("    <GenerateTargetFrameworkAttribute>false</GenerateTargetFrameworkAttribute>");
        sb.AppendLine("    <SuppressNETCoreSdkPreviewMessage>true</SuppressNETCoreSdkPreviewMessage>");
        sb.AppendLine("  </PropertyGroup>");

        // 引擎引用（相对路径 HintPath）
        var engineRefs = _engineResolver.Resolve(assembly.References);
        if (engineRefs.Count > 0)
        {
            sb.AppendLine("  <ItemGroup>");
            foreach (var engineRef in engineRefs)
            {
                sb.AppendLine($"    <Reference Include=\"{engineRef.Name}\">");
                sb.AppendLine($"      <HintPath>{engineRef.RelativeDllPath}</HintPath>");
                sb.AppendLine("      <Private>false</Private>");
                sb.AppendLine("    </Reference>");
            }
            sb.AppendLine("  </ItemGroup>");
        }

        // 源文件 glob
        sb.AppendLine("  <ItemGroup>");
        foreach (var glob in assembly.SourceGlobs)
        {
            sb.AppendLine($"    <Compile Include=\"{glob}\" />");
        }
        sb.AppendLine("  </ItemGroup>");

        sb.AppendLine("</Project>");

        return sb.ToString();
    }

    /// <summary>生成 .sln 内容（确定性 GUID）。</summary>
    private string GenerateSlnContent(IReadOnlyList<ScriptAssemblyDefinition> assemblies, IReadOnlyList<string> csprojFileNames)
    {
        var sb = new StringBuilder();

        sb.AppendLine("Microsoft Visual Studio Solution File, Format Version 12.00");
        sb.AppendLine("# Visual Studio Version 17");
        sb.AppendLine("VisualStudioVersion = 17.0.31903.59");
        sb.AppendLine("MinimumVisualStudioVersion = 10.0.40219.1");

        // 每个 csproj 一个 Project 条目
        for (int i = 0; i < assemblies.Count; i++)
        {
            var projectGuid = GenerateDeterministicGuid(assemblies[i].Name)
                .ToString("B").ToUpperInvariant();
            sb.AppendLine($"Project(\"{{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}}\") = \"{assemblies[i].Name}\", \"{csprojFileNames[i]}\", \"{projectGuid}\"");
            sb.AppendLine("EndProject");
        }

        sb.AppendLine("Global");

        // SolutionConfigurationPlatforms
        sb.AppendLine("\tGlobalSection(SolutionConfigurationPlatforms) = preSolution");
        sb.AppendLine("\t\tDebug|Any CPU = Debug|Any CPU");
        sb.AppendLine("\t\tRelease|Any CPU = Release|Any CPU");
        sb.AppendLine("\tEndGlobalSection");

        // ProjectConfigurationPlatforms
        sb.AppendLine("\tGlobalSection(ProjectConfigurationPlatforms) = postSolution");
        for (int i = 0; i < assemblies.Count; i++)
        {
            var projectGuid = GenerateDeterministicGuid(assemblies[i].Name)
                .ToString("B").ToUpperInvariant();
            sb.AppendLine($"\t\t{projectGuid}.Debug|Any CPU.ActiveCfg = Debug|Any CPU");
            sb.AppendLine($"\t\t{projectGuid}.Debug|Any CPU.Build.0 = Debug|Any CPU");
            sb.AppendLine($"\t\t{projectGuid}.Release|Any CPU.ActiveCfg = Release|Any CPU");
            sb.AppendLine($"\t\t{projectGuid}.Release|Any CPU.Build.0 = Release|Any CPU");
        }
        sb.AppendLine("\tEndGlobalSection");

        sb.AppendLine("EndGlobal");

        return sb.ToString();
    }

    /// <summary>基于名称生成确定性 GUID（MD5）。</summary>
    private static Guid GenerateDeterministicGuid(string name)
    {
        var hash = MD5.HashData(Encoding.UTF8.GetBytes($"Neverness.ScriptProject.{name}"));
        return new Guid(hash);
    }

    /// <summary>内容不变不写入。</summary>
    private static void WriteIfChanged(string path, string content)
    {
        try
        {
            if (File.Exists(path))
            {
                var existing = File.ReadAllText(path);
                if (existing == content)
                    return;
            }

            File.WriteAllText(path, content);
        }
        catch (IOException ex)
        {
            Console.WriteLine($"[ScriptProjectGenerator] WriteIfChanged failed: {path}: {ex.Message}");
        }
    }
}
