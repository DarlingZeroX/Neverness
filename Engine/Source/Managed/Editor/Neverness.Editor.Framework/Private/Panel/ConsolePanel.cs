namespace Neverness.Editor.Framework.Private.Panel;

/// <summary>
/// 主控台日誌面板模型（緩衝最近訊息供 Editor UI 綁定）。
/// </summary>
public sealed class ConsolePanel : EditorPanel
{
	private readonly List<string> _lines = [];
	private const int MaxLines = 512;

	/// <summary>建立主控台面板。</summary>
	public ConsolePanel() : base("console", "Console")
	{
	}

	/// <summary>日誌行（唯讀）。</summary>
	public IReadOnlyList<string> Lines => _lines;

	/// <summary>追加一行日誌。</summary>
	public void Log(string message)
	{
		ArgumentNullException.ThrowIfNull(message);
		_lines.Add(message);
		if (_lines.Count > MaxLines)
		{
			_lines.RemoveAt(0);
		}
	}

	/// <summary>清空緩衝。</summary>
	public void Clear() => _lines.Clear();
}
