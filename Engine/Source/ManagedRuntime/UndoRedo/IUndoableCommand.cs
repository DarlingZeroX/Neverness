namespace Neverness.Managed.UndoRedo;

/// <summary>
/// 可撤銷/重做之編輯器命令契約。
/// </summary>
public interface IUndoableCommand
{
	/// <summary>執行（Redo 路徑）。</summary>
	void Execute();

	/// <summary>撤銷。</summary>
	void Undo();

	/// <summary>人類可讀描述。</summary>
	string Description { get; }
}
