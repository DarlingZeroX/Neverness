// ============================================================================
// DisallowMultipleComponentAttribute.cs - 禁止多组件特性
// ============================================================================
// 标记脚本在同一 Entity 上只能添加一个实例。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 禁止多组件特性：标记脚本在同一 Entity 上只能添加一个实例。
/// </summary>
/// <remarks>
/// 使用示例：
/// <code>
/// [DisallowMultipleComponent]
/// public class PlayerController : EntityBehaviour
/// {
///     // 同一 Entity 上只能添加一个 PlayerController
/// }
/// </code>
/// </remarks>
[AttributeUsage(AttributeTargets.Class, Inherited = true, AllowMultiple = false)]
public sealed class DisallowMultipleComponentAttribute : Attribute
{
}
