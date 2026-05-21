namespace Neverness.Editor.Framework.UndoRedo;

/// <summary>
/// 可撤销/重做之编辑器命令契约。
/// </summary>
public interface IUndoableCommand
{
	/// <summary>执行（Redo 路径）。</summary>
	void Execute();

	/// <summary>撤销。</summary>
	void Undo();

	/// <summary>人类可读描述。</summary>
	string Description { get; }
}
