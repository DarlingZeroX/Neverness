// ============================================================================
// RequireComponentAttribute.cs - 要求组件特性
// ============================================================================
// 标记脚本需要依赖的组件，添加脚本时会自动添加依赖组件。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 要求组件特性：标记脚本需要依赖的组件。
/// </summary>
/// <remarks>
/// 添加脚本时会自动添加依赖组件。
///
/// 使用示例：
/// <code>
/// [RequireComponent(typeof(TransformComponent))]
/// public class PlayerController : EntityBehaviour
/// {
///     // 添加此脚本时会自动添加 TransformComponent
/// }
/// </code>
/// </remarks>
[AttributeUsage(AttributeTargets.Class, Inherited = true, AllowMultiple = true)]
public sealed class RequireComponentAttribute : Attribute
{
    /// <summary>依赖的组件类型。</summary>
    public Type RequiredType { get; }

    /// <summary>创建要求组件特性。</summary>
    /// <param name="requiredType">依赖的组件类型。</param>
    public RequireComponentAttribute(Type requiredType)
    {
        RequiredType = requiredType ?? throw new ArgumentNullException(nameof(requiredType));
    }
}
