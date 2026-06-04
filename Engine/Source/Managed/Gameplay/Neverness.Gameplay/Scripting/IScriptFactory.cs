// ============================================================================
// IScriptFactory.cs - 脚本工厂接口
// ============================================================================
// 创建 EntityBehaviour 实例的抽象。
// 当前实现：Activator.CreateInstance。
// 未来可替换为 DI Container / Object Pool。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 脚本工厂——创建 EntityBehaviour 实例的抽象。
/// </summary>
public interface IScriptFactory
{
    /// <summary>
    /// 创建指定类型的 EntityBehaviour 实例。
    /// </summary>
    /// <param name="scriptType">脚本类型（必须继承 EntityBehaviour）。</param>
    /// <returns>创建的实例，失败返回 null。</returns>
    EntityBehaviour? Create(Type scriptType);
}
