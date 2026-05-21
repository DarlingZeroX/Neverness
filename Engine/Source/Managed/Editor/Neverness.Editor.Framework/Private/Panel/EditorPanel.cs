//using Neverness.Managed.Engine;

namespace Neverness.Editor.Framework.Private.Panel;

/// <summary>
/// 编辑器面板抽象基底（Hierarchy、Inspector、Console 等）。
/// </summary>
public abstract class EditorPanel
{
	/// <summary>面板唯一识别码。</summary>
	public string Id { get; }

	/// <summary>显示标题。</summary>
	public string Title { get; }

	/// <summary>面板是否可见。</summary>
	public bool Visible { get; set; } = true;

	/// <summary>建立面板。</summary>
	protected EditorPanel(string id, string title)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(id);
		ArgumentException.ThrowIfNullOrWhiteSpace(title);
		Id = id;
		Title = title;
	}

	/// <summary>选取变更回调；衍生类别可覆写以刷新 UI 模型。</summary>
	//public virtual void OnSelectionChanged(NNEntityHandle? selectedEntity)
	//{
	//}
}
