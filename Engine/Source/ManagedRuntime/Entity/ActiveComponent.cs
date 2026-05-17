namespace Neverness.Managed.Entity;

/// <summary>
/// 啟用旗標：供遊戲邏輯、系統迭代時快速過濾「暫停參與」之實體；<b>不</b>改變 <see cref="EntityHandle"/> 或 <see cref="EntityWorld.IsAlive"/> 語意。
/// </summary>
/// <remarks>銷毀實體仍須呼叫 <see cref="EntityWorld.Destroy"/>；僅關閉本旗標不會釋放槽位。</remarks>
public sealed class ActiveComponent : VGComponent
{
	/// <inheritdoc />
	public ActiveComponent(EntityHandle entity)
		: base(entity)
	{
		IsActive = true;
	}

	/// <summary>為 <c>false</c> 時，系統可選擇略過該實體（例如隱藏、暫停 AI）。</summary>
	public bool IsActive { get; set; }
}
