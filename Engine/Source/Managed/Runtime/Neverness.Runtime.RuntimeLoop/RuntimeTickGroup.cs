namespace Neverness.Runtime.RuntimeLoop;

/// <summary>
/// 与 Native <c>visiongal::engine::RuntimeTickGroup</c> 语义对齐的托管 Tick 分组（<b>P0-1</b> <see cref="ManagedRuntimeScheduler"/>）。
/// 每帧顺序：<see cref="EarlyUpdate"/> →（0..N 次 <see cref="FixedUpdate"/>）→ <see cref="Update"/> → <see cref="LateUpdate"/> → <see cref="Render"/>。
/// </summary>
public enum RuntimeTickGroup : byte
{
	EarlyUpdate = 0,
	FixedUpdate,
	Update,
	LateUpdate,
	Render,
}
