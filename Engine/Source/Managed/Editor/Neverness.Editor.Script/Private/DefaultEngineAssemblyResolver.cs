// ============================================================================
// DefaultEngineAssemblyResolver.cs - 默认引擎程序集解析器
// ============================================================================
// 从引擎构建输出目录查找 DLL，返回相对路径 HintPath。
// ============================================================================

namespace Neverness.Editor.Script.Private;

/// <summary>
/// 默认引擎程序集解析器——从引擎构建输出目录查找 DLL。
/// </summary>
public sealed class DefaultEngineAssemblyResolver : IEngineAssemblyResolver
{
    private readonly string _engineBinariesDir;
    private readonly string _projectRoot;

    /// <summary>创建默认解析器。</summary>
    /// <param name="engineBinariesDir">引擎 DLL 目录（OS 绝对路径）。</param>
    /// <param name="projectRoot">项目根目录（OS 绝对路径，用于计算相对路径）。</param>
    public DefaultEngineAssemblyResolver(string engineBinariesDir, string projectRoot)
    {
        _engineBinariesDir = engineBinariesDir;
        _projectRoot = projectRoot;
    }

    /// <inheritdoc />
    public IReadOnlyList<EngineAssemblyReference> Resolve(IReadOnlyList<string> assemblyNames)
    {
        var result = new List<EngineAssemblyReference>();

        foreach (var name in assemblyNames)
        {
            var dllPath = Path.Combine(_engineBinariesDir, $"{name}.dll");
            if (File.Exists(dllPath))
            {
                // 计算相对于项目根目录的路径
                var relativePath = Path.GetRelativePath(_projectRoot, dllPath).Replace('/', '\\');
                result.Add(new EngineAssemblyReference
                {
                    Name = name,
                    RelativeDllPath = relativePath
                });
            }
            else
            {
                Console.WriteLine($"[DefaultEngineAssemblyResolver] Engine assembly not found: {dllPath}");
            }
        }

        return result;
    }
}
