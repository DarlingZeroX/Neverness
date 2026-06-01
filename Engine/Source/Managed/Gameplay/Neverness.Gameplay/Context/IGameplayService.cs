// ============================================================================
// IGameplayService.cs - Gameplay 服务接口
// ============================================================================
// Gameplay 服务的基础接口。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// Gameplay 服务接口。
/// </summary>
public interface IGameplayService
{
    /// <summary>服务名称。</summary>
    string Name => GetType().Name;

    /// <summary>初始化服务。</summary>
    void Initialize();

    /// <summary>关闭服务。</summary>
    void Shutdown();
}
