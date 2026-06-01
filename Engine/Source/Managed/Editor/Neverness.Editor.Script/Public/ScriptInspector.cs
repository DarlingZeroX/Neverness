// ============================================================================
// ScriptInspector.cs - 脚本检查器
// ============================================================================
// 在 Inspector 中显示和编辑 Entity 上挂载的 EntityBehaviour 脚本。
// ============================================================================

using Neverness.Gameplay;

namespace Neverness.Editor.Script.Public;

/// <summary>
/// 脚本检查器：在 Inspector 中显示和编辑 Entity 上挂载的 EntityBehaviour 脚本。
/// </summary>
public sealed class ScriptInspector
{
    // ========================================================================
    // 内部类型
    // ========================================================================

    /// <summary>脚本字段信息。</summary>
    public sealed class FieldInfo
    {
        /// <summary>字段名称。</summary>
        public required string Name { get; init; }

        /// <summary>字段类型。</summary>
        public required Type FieldType { get; init; }

        /// <summary>当前值。</summary>
        public object? Value { get; set; }

        /// <summary>是否在 Inspector 中隐藏。</summary>
        public bool IsHidden { get; init; }

        /// <summary>标题文本。</summary>
        public string? Header { get; init; }

        /// <summary>是否可编辑。</summary>
        public bool IsEditable { get; init; } = true;
    }

    // ========================================================================
    // 公共方法
    // ========================================================================

    /// <summary>
    /// 获取 Entity 上所有 Behaviour 的字段信息。
    /// </summary>
    /// <param name="entity">Entity 实例。</param>
    /// <returns>Behaviour 名称 → 字段列表的映射。</returns>
    public static Dictionary<string, List<FieldInfo>> GetBehaviourFields(Entity entity)
    {
        ArgumentNullException.ThrowIfNull(entity);

        var result = new Dictionary<string, List<FieldInfo>>();
        var behaviours = entity.GetBehaviours();

        foreach (var behaviour in behaviours)
        {
            var fields = GetFieldsForBehaviour(behaviour);
            if (fields.Count > 0)
            {
                result[behaviour.GetType().Name] = fields;
            }
        }

        return result;
    }

    /// <summary>
    /// 获取单个 Behaviour 的字段信息。
    /// </summary>
    /// <param name="behaviour">Behaviour 实例。</param>
    /// <returns>字段列表。</returns>
    public static List<FieldInfo> GetFieldsForBehaviour(EntityBehaviour behaviour)
    {
        ArgumentNullException.ThrowIfNull(behaviour);

        var result = new List<FieldInfo>();
        var type = behaviour.GetType();
        var fields = type.GetFields(System.Reflection.BindingFlags.Instance |
                                    System.Reflection.BindingFlags.Public |
                                    System.Reflection.BindingFlags.NonPublic);

        foreach (var field in fields)
        {
            // 跳过 EntityBehaviour 基类的字段
            if (field.DeclaringType == typeof(EntityBehaviour))
                continue;

            // 跳过编译器生成的字段
            if (field.Name.StartsWith("<", StringComparison.Ordinal))
                continue;

            // 检查是否标记了 [HideInInspector]
            var isHidden = Attribute.IsDefined(field, typeof(HideInInspectorAttribute));

            // 获取 [Header] 属性
            var headerAttr = field.GetCustomAttributes(typeof(HeaderAttribute), false)
                .FirstOrDefault() as HeaderAttribute;

            result.Add(new FieldInfo
            {
                Name = field.Name,
                FieldType = field.FieldType,
                Value = field.GetValue(behaviour),
                IsHidden = isHidden,
                Header = headerAttr?.Header,
                IsEditable = !field.IsInitOnly
            });
        }

        return result;
    }

    /// <summary>
    /// 设置 Behaviour 的字段值。
    /// </summary>
    /// <param name="behaviour">Behaviour 实例。</param>
    /// <param name="fieldName">字段名称。</param>
    /// <param name="value">新值。</param>
    /// <returns>是否设置成功。</returns>
    public static bool SetFieldValue(EntityBehaviour behaviour, string fieldName, object? value)
    {
        ArgumentNullException.ThrowIfNull(behaviour);
        ArgumentException.ThrowIfNullOrWhiteSpace(fieldName);

        var type = behaviour.GetType();
        var field = type.GetField(fieldName, System.Reflection.BindingFlags.Instance |
                                             System.Reflection.BindingFlags.Public |
                                             System.Reflection.BindingFlags.NonPublic);

        if (field is null || field.IsInitOnly)
        {
            return false;
        }

        try
        {
            field.SetValue(behaviour, value);
            return true;
        }
        catch
        {
            return false;
        }
    }

    /// <summary>
    /// 添加脚本组件到 Entity。
    /// </summary>
    /// <param name="entity">Entity 实例。</param>
    /// <param name="scriptType">脚本类型。</param>
    /// <returns>新创建的 Behaviour 实例。</returns>
    public static EntityBehaviour? AddScriptComponent(Entity entity, Type scriptType)
    {
        ArgumentNullException.ThrowIfNull(entity);
        ArgumentNullException.ThrowIfNull(scriptType);

        // 检查是否继承了 EntityBehaviour
        if (!typeof(EntityBehaviour).IsAssignableFrom(scriptType))
        {
            return null;
        }

        // 检查是否标记了 [DisallowMultipleComponent]
        if (Attribute.IsDefined(scriptType, typeof(DisallowMultipleComponentAttribute)))
        {
            var existing = entity.GetBehaviours().FirstOrDefault(b => b.GetType() == scriptType);
            if (existing is not null)
            {
                return existing;
            }
        }

        // 创建实例
        var behaviour = Activator.CreateInstance(scriptType) as EntityBehaviour;
        if (behaviour is null)
        {
            return null;
        }

        behaviour.Entity = entity;
        ScriptBehaviourScheduler.Instance?.Register(entity, behaviour);

        return behaviour;
    }

    /// <summary>
    /// 移除脚本组件。
    /// </summary>
    /// <param name="entity">Entity 实例。</param>
    /// <param name="behaviour">要移除的 Behaviour。</param>
    /// <returns>是否移除成功。</returns>
    public static bool RemoveScriptComponent(Entity entity, EntityBehaviour behaviour)
    {
        ArgumentNullException.ThrowIfNull(entity);
        ArgumentNullException.ThrowIfNull(behaviour);

        ScriptBehaviourScheduler.Instance?.MarkForDestroy(behaviour);
        return true;
    }
}
