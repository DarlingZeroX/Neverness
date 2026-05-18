using Neverness.Managed.Object;

namespace VisionGal.Managed.Editor;

/// <summary>
/// 編輯器主殼層：管理面板生命週期、選取與命令分發之協調入口。
/// </summary>
public sealed class EditorShell
{
	private readonly List<EditorPanel> _panels = [];
	private readonly SelectionService _selection = new();
	private readonly CommandRegistry _commands = new();

	/// <summary>目前停靠佈局。</summary>
	public DockingLayout Layout { get; } = new();

	/// <summary>選取服務。</summary>
	public SelectionService Selection => _selection;

	/// <summary>命令註冊表。</summary>
	public CommandRegistry Commands => _commands;

	/// <summary>已註冊面板（唯讀）。</summary>
	public IReadOnlyList<EditorPanel> Panels => _panels;

	/// <summary>註冊面板並分配停靠區域。</summary>
	public void RegisterPanel(EditorPanel panel, string dockZone = "main")
	{
		ArgumentNullException.ThrowIfNull(panel);
		_panels.Add(panel);
		Layout.Place(panel.Id, dockZone);
	}

	/// <summary>執行具名命令。</summary>
	public bool ExecuteCommand(string commandId) => _commands.TryExecute(commandId);

	/// <summary>更新選取並通知面板。</summary>
	public void SetSelection(VGObjectId? selectedId)
	{
		_selection.SetSelected(selectedId);
		foreach (var panel in _panels)
		{
			panel.OnSelectionChanged(_selection.SelectedId);
		}
	}
}
