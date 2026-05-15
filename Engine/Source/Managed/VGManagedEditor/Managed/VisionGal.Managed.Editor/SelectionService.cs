using VisionGal.Managed.Object;

namespace VisionGal.Managed.Editor;

/// <summary>
/// 編輯器全域選取狀態（目前選中之 <see cref="VGObjectId"/>）。
/// </summary>
public sealed class SelectionService
{
	/// <summary>目前選取 Id；null 表示無選取。</summary>
	public VGObjectId? SelectedId { get; private set; }

	/// <summary>設定選取。</summary>
	public void SetSelected(VGObjectId? id) => SelectedId = id;

	/// <summary>清除選取。</summary>
	public void Clear() => SelectedId = null;

	/// <summary>是否有有效選取。</summary>
	public bool HasSelection => SelectedId is { IsValid: true };
}
