namespace Neverness.Runtime.Scene;

/// <summary>
/// Managed System Tick 分组——与 Native <c>NNSceneTickGroup</c> 数值对齐。
/// Native System 在 Bridge.TickSystems 内部按其 TickGroup 调度；
/// Managed System 由 <see cref="SceneSystemScheduler"/> 按此枚举顺序调度。
/// </summary>
public enum TickGroup : byte
{
    EarlyUpdate = 0,
    FixedUpdate = 1,
    Update = 2,
    LateUpdate = 3,
    Render = 4,
}
