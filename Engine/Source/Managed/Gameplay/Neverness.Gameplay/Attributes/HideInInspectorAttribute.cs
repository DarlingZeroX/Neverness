// ============================================================================
// HideInInspectorAttribute.cs - 隐藏在 Inspector
// ============================================================================
// 标记字段在 Inspector 中不显示。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 隐藏在 Inspector 特性：标记字段在 Inspector 中不显示。
/// </summary>
/// <remarks>
/// 使用示例：
/// <code>
/// public class PlayerController : EntityBehaviour
/// {
///     [HideInInspector]
///     public float internalValue;
///
///     public float speed;  // 此字段会在 Inspector 中显示
/// }
/// </code>
/// </remarks>
[AttributeUsage(AttributeTargets.Field, Inherited = true, AllowMultiple = false)]
public sealed class HideInInspectorAttribute : Attribute
{
}
