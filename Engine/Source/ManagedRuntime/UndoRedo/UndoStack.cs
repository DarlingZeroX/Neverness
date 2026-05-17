namespace VisionGal.Managed.UndoRedo;

/// <summary>
/// 雙棧 Undo/Redo 管理器。
/// </summary>
public sealed class UndoStack
{
	private readonly Stack<IUndoableCommand> _undo = new();
	private readonly Stack<IUndoableCommand> _redo = new();

	/// <summary>是否可 Undo。</summary>
	public bool CanUndo => _undo.Count > 0;

	/// <summary>是否可 Redo。</summary>
	public bool CanRedo => _redo.Count > 0;

	/// <summary>執行並壓入 Undo 棧；清空 Redo 棧。</summary>
	public void PushAndExecute(IUndoableCommand command)
	{
		ArgumentNullException.ThrowIfNull(command);
		command.Execute();
		_undo.Push(command);
		_redo.Clear();
	}

	/// <summary>撤銷上一命令。</summary>
	public bool TryUndo()
	{
		if (_undo.Count == 0)
		{
			return false;
		}

		var cmd = _undo.Pop();
		cmd.Undo();
		_redo.Push(cmd);
		return true;
	}

	/// <summary>重做上一撤銷。</summary>
	public bool TryRedo()
	{
		if (_redo.Count == 0)
		{
			return false;
		}

		var cmd = _redo.Pop();
		cmd.Execute();
		_undo.Push(cmd);
		return true;
	}
}
