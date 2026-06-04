// ============================================================================
// DefaultScriptFactory.cs - 默认脚本工厂
// ============================================================================
// 使用 Activator.CreateInstance 创建 EntityBehaviour 实例。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 默认脚本工厂——使用 Activator.CreateInstance 创建实例。
/// </summary>
public sealed class DefaultScriptFactory : IScriptFactory
{
    /// <inheritdoc />
    public EntityBehaviour? Create(Type scriptType)
    {
        try
        {
            return Activator.CreateInstance(scriptType) as EntityBehaviour;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[DefaultScriptFactory] Failed to create {scriptType.Name}: {ex.Message}");
            return null;
        }
    }
}
