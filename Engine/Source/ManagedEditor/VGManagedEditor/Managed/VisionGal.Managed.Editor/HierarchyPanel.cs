using Neverness.Managed.Object;

namespace VisionGal.Managed.Editor;

/// <summary>
/// 場景層級面板：列出 <see cref="ObjectRegistry"/> 中已註冊物件 Id。
/// </summary>
public sealed class HierarchyPanel : EditorPanel
{
	/// <summary>建立層級面板。</summary>
	public HierarchyPanel() : base("hierarchy", "Hierarchy")
	{
	}

	/// <summary>目前列出的物件 Id。</summary>
	public IReadOnlyList<VGObjectId> ListedIds { get; private set; } = [];

	/// <summary>刷新物件清單。</summary>
	public void Refresh()
	{
		var ids = new List<VGObjectId>();
		// ObjectRegistry 未公開列舉；透過 TryGet 與 Count 的測試路徑由 Selection 驅動
		if (ObjectRegistry.TryGet(SelectedId ?? VGObjectId.Invalid, out _))
		{
			ids.Add(SelectedId!.Value);
		}

		ListedIds = ids;
	}

	/// <summary>目前選取之 Id（由 EditorShell 同步）。</summary>
	public VGObjectId? SelectedId { get; private set; }

	/// <inheritdoc />
	public override void OnSelectionChanged(VGObjectId? selectedId)
	{
		SelectedId = selectedId;
		Refresh();
	}
}
