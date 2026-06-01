// ============================================================================
// ScriptRegistry.cs - 脚本注册表
// ============================================================================
// 管理所有 EntityBehaviour 类型的注册与查找。
// 所有模式（Editor/Dev/Release）都使用 Source Generator 驱动注册。
// ============================================================================

using System.Reflection;

namespace Neverness.Gameplay;

/// <summary>
/// 脚本注册表：管理所有 EntityBehaviour 类型的注册与查找。
/// </summary>
/// <remarks>
/// ⚠️ 所有模式都使用 Source Generator 驱动 Script Registry。
/// 不使用运行时反射扫描（NativeAOT 兼容）。
/// </remarks>
public sealed class ScriptRegistry
{
    // ========================================================================
    // 内部类型
    // ========================================================================

    /// <summary>脚本类型信息。</summary>
    public sealed class ScriptTypeInfo
    {
        /// <summary>类型名称。</summary>
        public required string Name { get; init; }

        /// <summary>完整类型名。</summary>
        public required string FullName { get; init; }

        /// <summary>类型对象。</summary>
        public required Type Type { get; init; }

        /// <summary>是否禁用。</summary>
        public bool IsDisabled { get; init; }

        /// <summary>显示名称（用于 Inspector）。</summary>
        public string DisplayName { get; init; }

        /// <summary>分类标签。</summary>
        public string Category { get; init; } = "Default";

        /// <summary>脚本类型 ID（FNV-1a hash）。</summary>
        public ulong TypeId { get; init; }
    }

    // ========================================================================
    // 内部状态
    // ========================================================================

    /// <summary>类型名 → 类型信息。</summary>
    private readonly Dictionary<string, ScriptTypeInfo> _scriptsByName = new();

    /// <summary>Type → 类型信息。</summary>
    private readonly Dictionary<Type, ScriptTypeInfo> _scriptsByType = new();

    /// <summary>TypeId → 类型信息。</summary>
    private readonly Dictionary<ulong, ScriptTypeInfo> _scriptsByTypeId = new();

    // ========================================================================
    // 公共属性
    // ========================================================================

    /// <summary>已注册的脚本类型数量。</summary>
    public int Count => _scriptsByType.Count;

    // ========================================================================
    // 注册方法
    // ========================================================================

    /// <summary>
    /// 注册脚本类型（由 Source Generator 调用）。
    /// </summary>
    /// <typeparam name="T">脚本类型（必须继承 EntityBehaviour）。</typeparam>
    /// <remarks>
    /// ⚠️ 所有模式都使用此方法注册，不使用反射扫描。
    /// Source Generator 会在编译时生成调用此方法的代码。
    /// </remarks>
    public void Register<T>() where T : EntityBehaviour
    {
        Register(typeof(T));
    }

    /// <summary>
    /// 注册脚本类型（由 Source Generator 调用）。
    /// </summary>
    /// <param name="type">脚本类型。</param>
    public void Register(Type type)
    {
        ArgumentNullException.ThrowIfNull(type);

        // 检查是否已注册
        if (_scriptsByType.ContainsKey(type))
        {
            return;
        }

        // 计算类型 ID（FNV-1a hash）
        var typeId = CalculateTypeId(type.FullName!);

        // 创建类型信息
        var info = new ScriptTypeInfo
        {
            Name = type.Name,
            FullName = type.FullName!,
            Type = type,
            TypeId = typeId,
            DisplayName = type.Name,  // 默认使用类型名
            Category = "Default"
        };

        // 注册
        _scriptsByName[info.FullName] = info;
        _scriptsByType[type] = info;
        _scriptsByTypeId[typeId] = info;
    }

    // ========================================================================
    // 查找方法
    // ========================================================================

    /// <summary>
    /// 按完整类型名查找脚本类型。
    /// </summary>
    /// <param name="fullName">完整类型名。</param>
    /// <returns>脚本类型信息，未找到时返回 null。</returns>
    public ScriptTypeInfo? FindByName(string fullName)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(fullName);
        return _scriptsByName.GetValueOrDefault(fullName);
    }

    /// <summary>
    /// 按 Type 查找脚本类型。
    /// </summary>
    /// <param name="type">脚本类型。</param>
    /// <returns>脚本类型信息，未找到时返回 null。</returns>
    public ScriptTypeInfo? FindByType(Type type)
    {
        ArgumentNullException.ThrowIfNull(type);
        return _scriptsByType.GetValueOrDefault(type);
    }

    /// <summary>
    /// 按 TypeId（FNV-1a hash）查找脚本类型。
    /// </summary>
    /// <param name="typeId">脚本类型 ID。</param>
    /// <returns>脚本类型信息，未找到时返回 null。</returns>
    public ScriptTypeInfo? FindByTypeId(ulong typeId)
    {
        return _scriptsByTypeId.GetValueOrDefault(typeId);
    }

    /// <summary>
    /// 获取所有注册的脚本类型。
    /// </summary>
    /// <returns>脚本类型信息集合。</returns>
    public IReadOnlyCollection<ScriptTypeInfo> GetAllScripts()
    {
        return _scriptsByType.Values;
    }

    /// <summary>
    /// 检查是否已注册指定类型。
    /// </summary>
    /// <param name="type">脚本类型。</param>
    /// <returns>是否已注册。</returns>
    public bool IsRegistered(Type type)
    {
        return _scriptsByType.ContainsKey(type);
    }

    // ========================================================================
    // 清理方法
    // ========================================================================

    /// <summary>清除所有注册。</summary>
    public void Clear()
    {
        _scriptsByName.Clear();
        _scriptsByType.Clear();
        _scriptsByTypeId.Clear();
    }

    // ========================================================================
    // 内部方法
    // ========================================================================

    /// <summary>
    /// 计算类型 ID（FNV-1a hash）。
    /// </summary>
    /// <param name="fullName">完整类型名。</param>
    /// <returns>FNV-1a hash 值。</returns>
    private static ulong CalculateTypeId(string fullName)
    {
        // FNV-1a 64-bit hash
        const ulong FNV_OFFSET_BASIS = 14695981039346656037UL;
        const ulong FNV_PRIME = 1099511628211UL;

        ulong hash = FNV_OFFSET_BASIS;
        foreach (var c in fullName)
        {
            hash ^= (byte)c;
            hash *= FNV_PRIME;
        }
        return hash;
    }
}
