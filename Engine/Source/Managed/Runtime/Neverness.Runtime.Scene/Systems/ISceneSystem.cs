namespace Neverness.Runtime.Scene;

/// <summary>
/// System 基础标记接口——所有 Managed Scene System 的根接口。
/// 实现类可选择性地实现 <see cref="ISystemInitialize"/>、<see cref="ISystemShutdown"/>、
/// <see cref="ISystemTick"/>、<see cref="ISystemFixedTick"/>、<see cref="ISystemLateTick"/>。
/// </summary>
public interface ISceneSystem
{
    /// <summary>系统名称（用于调试和日志）。默认取类型名。</summary>
    string Name => GetType().Name;
}
