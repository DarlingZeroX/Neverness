// ============================================================================
// ScriptAssetIndex.cs - 脚本资产索引
// ============================================================================
// Editor 层缓存：GUID ↔ FullName ↔ ScriptTypeId 映射。
// 构建时机：Editor 启动扫描（Phase A）+ ScriptRegistry Ready 时补全 TypeId（Phase B）。
//
// 核心规则：
// - ScriptTypeId 只能来源于 ScriptRegistry（运行时权威）
// - 本类不做 TypeId 计算，只做映射查询
// ============================================================================

using System.Text.RegularExpressions;
using Neverness.Editor.Assets;
using Neverness.Gameplay;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS.Public;

namespace Neverness.Editor.Script.Private;

/// <summary>
/// 脚本资产索引——GUID ↔ FullName ↔ ScriptTypeId 映射缓存。
///
/// 双阶段初始化：
/// - Phase A（Editor 启动）：扫描所有 .cs 资产，建立 GUID → FullName
/// - Phase B（ScriptRegistry Ready）：FullName → ScriptTypeId 补全
/// </summary>
public sealed class ScriptAssetIndex
{
    // ========================================================================
    // 单例
    // ========================================================================

    /// <summary>全局实例。</summary>
    public static ScriptAssetIndex Instance { get; } = new();

    // ========================================================================
    // 内部状态
    // ========================================================================

    /// <summary>GUID → FullName（从 .cs 源文件解析）。</summary>
    private readonly Dictionary<GUID, string> _guidToFullName = new();

    /// <summary>FullName → ScriptTypeId（从 ScriptRegistry 查询）。</summary>
    private readonly Dictionary<string, ulong> _fullNameToTypeId = new();

    /// <summary>GUID → ScriptTypeId（组合查询缓存）。</summary>
    private readonly Dictionary<GUID, ulong> _guidToTypeId = new();

    // ========================================================================
    // 公共查询 API
    // ========================================================================

    /// <summary>根据资产 GUID 查询 ScriptTypeId。</summary>
    /// <param name="assetGuid">资产 GUID。</param>
    /// <param name="scriptTypeId">输出的 ScriptTypeId。</param>
    /// <returns>是否找到。</returns>
    public bool TryGetScriptTypeId(GUID assetGuid, out ulong scriptTypeId)
    {
        return _guidToTypeId.TryGetValue(assetGuid, out scriptTypeId);
    }

    /// <summary>根据资产 GUID 查询 FullName。</summary>
    /// <param name="assetGuid">资产 GUID。</param>
    /// <param name="fullName">输出的 FullName。</param>
    /// <returns>是否找到。</returns>
    public bool TryGetFullName(GUID assetGuid, out string fullName)
    {
        return _guidToFullName.TryGetValue(assetGuid, out fullName);
    }

    /// <summary>根据 FullName 查询 ScriptTypeId。</summary>
    /// <param name="fullName">脚本类 FullName。</param>
    /// <param name="scriptTypeId">输出的 ScriptTypeId。</param>
    /// <returns>是否找到。</returns>
    public bool TryGetTypeIdByFullName(string fullName, out ulong scriptTypeId)
    {
        return _fullNameToTypeId.TryGetValue(fullName, out scriptTypeId);
    }

    // ========================================================================
    // 注册 / 更新 API
    // ========================================================================

    /// <summary>注册/更新一个脚本资产映射。</summary>
    /// <param name="assetGuid">资产 GUID。</param>
    /// <param name="fullName">脚本类 FullName。</param>
    public void Register(GUID assetGuid, string fullName)
    {
        _guidToFullName[assetGuid] = fullName;

        // 尝试从 ScriptRegistry 查询 TypeId
        var registry = GameplayContext.Current?.ScriptRegistry;
        if (registry != null)
        {
            var typeInfo = registry.FindByName(fullName);
            if (typeInfo != null)
            {
                _fullNameToTypeId[fullName] = typeInfo.TypeId;
                _guidToTypeId[assetGuid] = typeInfo.TypeId;
            }
        }
    }

    /// <summary>移除一个脚本资产映射。</summary>
    /// <param name="assetGuid">资产 GUID。</param>
    public void Unregister(GUID assetGuid)
    {
        if (_guidToFullName.TryGetValue(assetGuid, out var fullName))
        {
            _guidToFullName.Remove(assetGuid);
            _guidToTypeId.Remove(assetGuid);

            // 检查是否还有其他 GUID 引用同一 FullName
            var hasOther = _guidToFullName.ContainsValue(fullName);
            if (!hasOther)
            {
                _fullNameToTypeId.Remove(fullName);
            }
        }
    }

    // ========================================================================
    // 构建 API
    // ========================================================================

    /// <summary>
    /// Phase A：全量重建 GUID → FullName 映射。
    /// Editor 启动时调用，扫描所有 .cs 资产。
    /// </summary>
    public void RebuildAll()
    {
        _guidToFullName.Clear();
        _guidToTypeId.Clear();
        _fullNameToTypeId.Clear();

        // 遍历 EditorAssetDatabase 中所有 C# 脚本资产
        foreach (var guid in EditorAssetDatabase.AllAssets)
        {
            if (guid.IsZero)
                continue;

            // 检查是否是 .cs 资产
            var typeId = EditorAssetDatabase.GetTypeId(guid);
            if (typeId != AssetTypeId.CSharpScript)
                continue;

            // 获取 VFS 路径 → OS 文件路径
            if (!EditorAssetDatabase.TryGetPath(guid, out var virtualPath))
                continue;

            // VFS 路径 → OS 绝对路径
            var osPath = VFS.GetAbsolutePath(virtualPath.FullPath);
            if (string.IsNullOrEmpty(osPath) || !File.Exists(osPath))
                continue;

            // 读取源文件并解析 FullName
            try
            {
                var sourceCode = File.ReadAllText(osPath);
                var fullName = ParseScriptFullName(sourceCode);
                if (!string.IsNullOrEmpty(fullName))
                {
                    _guidToFullName[guid] = fullName;
                }
            }
            catch
            {
                // 文件读取失败，跳过
            }
        }

        // 尝试补全 TypeId
        RefreshAllTypeIds();
    }

    /// <summary>
    /// Phase B：从 ScriptRegistry 补全所有 FullName → TypeId 映射。
    /// ScriptRegistry Ready 时调用。
    /// </summary>
    public void RefreshAllTypeIds()
    {
        var registry = GameplayContext.Current?.ScriptRegistry;
        if (registry == null)
            return;

        _fullNameToTypeId.Clear();
        _guidToTypeId.Clear();

        foreach (var (assetGuid, fullName) in _guidToFullName)
        {
            var typeInfo = registry.FindByName(fullName);
            if (typeInfo != null)
            {
                _fullNameToTypeId[fullName] = typeInfo.TypeId;
                _guidToTypeId[assetGuid] = typeInfo.TypeId;
            }
        }
    }

    // ========================================================================
    // 内部方法
    // ========================================================================

    /// <summary>
    /// 从 .cs 源文件解析脚本类的 FullName。
    /// 策略：提取 namespace 和第一个 class 声明，拼接为 FullName。
    /// ⚠️ 仅用于 Index 构建（离线操作），不用于运行时决策。
    /// </summary>
    /// <param name="sourceCode">C# 源代码。</param>
    /// <returns>FullName，解析失败返回 null。</returns>
    private static string? ParseScriptFullName(string sourceCode)
    {
        // 提取 namespace（支持 file-scoped 和 block-scoped）
        string? ns = null;
        var nsMatch = Regex.Match(sourceCode, @"namespace\s+([\w.]+)");
        if (nsMatch.Success)
            ns = nsMatch.Groups[1].Value;

        // 提取第一个 class 名称（跳过注释中的 class）
        var classMatch = Regex.Match(sourceCode, @"(?<!//.*?)\bclass\s+(\w+)");
        if (!classMatch.Success)
            return null;

        string className = classMatch.Groups[1].Value;
        return string.IsNullOrEmpty(ns) ? className : $"{ns}.{className}";
    }
}
