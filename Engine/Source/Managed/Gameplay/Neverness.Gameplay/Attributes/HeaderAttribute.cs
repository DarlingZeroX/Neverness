// ============================================================================
// HeaderAttribute.cs - 标题特性
// ============================================================================
// 在 Inspector 中为字段添加标题。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 标题特性：在 Inspector 中为字段添加标题。
/// </summary>
/// <remarks>
/// 使用示例：
/// <code>
/// public class PlayerController : EntityBehaviour
/// {
///     [Header("Movement")]
///     public float speed = 5.0f;
///     public float jumpForce = 10.0f;
///
///     [Header("Combat")]
///     public float attackDamage = 10.0f;
/// }
/// </code>
/// </remarks>
[AttributeUsage(AttributeTargets.Field, Inherited = true, AllowMultiple = false)]
public sealed class HeaderAttribute : Attribute
{
    /// <summary>标题文本。</summary>
    public string Header { get; }

    /// <summary>创建标题特性。</summary>
    /// <param name="header">标题文本。</param>
    public HeaderAttribute(string header)
    {
        Header = header ?? throw new ArgumentNullException(nameof(header));
    }
}
