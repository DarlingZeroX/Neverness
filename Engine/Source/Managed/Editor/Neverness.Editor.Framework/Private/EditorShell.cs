//using Neverness.Managed.Engine;

using Neverness.Editor.Framework.Private.Panel;

namespace Neverness.Editor.Framework.Private;

/// <summary>
/// 编辑器主壳层：管理面板生命周期、选取与命令分发之协调入口。
/// </summary>
public sealed class UIInterface
{
	private readonly List<EditorPanel> _panels = [];
	private readonly SelectionService _selection = new();
	private readonly CommandRegistry _commands = new();

	/// <summary>目前停靠布局。</summary>
	public DockingLayout Layout { get; } = new();

	/// <summary>选取服务。</summary>
	public SelectionService Selection => _selection;

	/// <summary>命令注册表。</summary>
	public CommandRegistry Commands => _commands;

	/// <summary>已注册面板（只读）。</summary>
	public IReadOnlyList<EditorPanel> Panels => _panels;

	/// <summary>注册面板并分配停靠区域。</summary>
	public void RegisterPanel(EditorPanel panel, string dockZone = "main")
	{
		ArgumentNullException.ThrowIfNull(panel);
		_panels.Add(panel);
		Layout.Place(panel.Id, dockZone);
	}

	/// <summary>执行具名命令。</summary>
	public bool ExecuteCommand(string commandId) => _commands.TryExecute(commandId);

	/// <summary>更新选取并通知面板。</summary>
	//public void SetSelection(NNEntityHandle? selectedEntity)
	//{
	//	_selection.SetSelected(selectedEntity);
	//	foreach (var panel in _panels)
	//	{
	//		panel.OnSelectionChanged(_selection.SelectedEntity);
	//	}
	//}
}
