namespace Neverness.Runtime.Scene;

/// <summary>
/// System 依赖排序特性——声明当前 System 必须在指定 System 之后执行。
/// 可多次标注以声明多个依赖。
/// <example>
/// <code>
/// [SystemDependency(typeof(MovementSystem))]
/// public class CollisionSystem : ISystemTick { ... }
/// </code>
/// </example>
/// </summary>
[AttributeUsage(AttributeTargets.Class, Inherited = false, AllowMultiple = true)]
public sealed class SystemDependencyAttribute : Attribute
{
    /// <summary>依赖的 System 类型（当前 System 必须在其之后执行）。</summary>
    public Type DependsOn { get; }

    public SystemDependencyAttribute(Type dependsOn)
    {
        ArgumentNullException.ThrowIfNull(dependsOn);
        DependsOn = dependsOn;
    }
}
