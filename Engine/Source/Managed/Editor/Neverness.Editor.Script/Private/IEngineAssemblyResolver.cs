// ============================================================================
// IEngineAssemblyResolver.cs - 引擎程序集解析器接口
// ============================================================================
// 根据程序集名称查找 DLL 路径，返回相对路径 HintPath。
// 不写死绝对路径，支持 Debug/Release/Editor/Runtime 不同配置。
// ============================================================================

namespace Neverness.Editor.Script.Private;

/// <summary>
/// 引擎程序集引用信息。
/// </summary>
public sealed class EngineAssemblyReference
{
    /// <summary>程序集名称（如 Neverness.Gameplay）。</summary>
    public required string Name { get; init; }

    /// <summary>DLL 相对于项目根目录的路径（如 Engine/Binaries/Neverness.Gameplay.dll）。</summary>
    public required string RelativeDllPath { get; init; }
}

/// <summary>
/// 引擎程序集解析器——根据程序集名称查找 DLL 路径。
/// </summary>
public interface IEngineAssemblyResolver
{
    /// <summary>
    /// 解析指定名称列表的引擎程序集引用。
    /// </summary>
    /// <param name="assemblyNames">需要引用的程序集名称列表。</param>
    /// <returns>解析成功的引用列表。</returns>
    IReadOnlyList<EngineAssemblyReference> Resolve(IReadOnlyList<string> assemblyNames);
}
