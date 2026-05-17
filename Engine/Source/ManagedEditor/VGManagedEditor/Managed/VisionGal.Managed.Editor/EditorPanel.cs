using Neverness.Managed.Object;

namespace VisionGal.Managed.Editor;

/// <summary>
/// 編輯器面板抽象基底（Hierarchy、Inspector、Console 等）。
/// </summary>
public abstract class EditorPanel
{
	/// <summary>面板唯一識別碼。</summary>
	public string Id { get; }

	/// <summary>顯示標題。</summary>
	public string Title { get; }

	/// <summary>面板是否可見。</summary>
	public bool Visible { get; set; } = true;

	/// <summary>建立面板。</summary>
	protected EditorPanel(string id, string title)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(id);
		ArgumentException.ThrowIfNullOrWhiteSpace(title);
		Id = id;
		Title = title;
	}

	/// <summary>選取變更回呼；衍生類別可覆寫以刷新 UI 模型。</summary>
	public virtual void OnSelectionChanged(VGObjectId? selectedId)
	{
	}
}
