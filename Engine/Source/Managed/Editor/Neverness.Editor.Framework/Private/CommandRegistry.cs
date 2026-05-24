using Neverness.Editor.Framework.Interface;

namespace Neverness.Editor.Framework.Private;

/// <summary>
/// 編輯器命令註冊表與執行分發。
/// </summary>
public sealed class CommandRegistry : ICommandRegistry
{
	private readonly Dictionary<string, IEditorCommand> _commands = new(StringComparer.OrdinalIgnoreCase);

	/// <summary>註冊命令；重複 Id 時覆寫。</summary>
	public void Register(IEditorCommand command)
	{
		ArgumentNullException.ThrowIfNull(command);
		_commands[command.CommandId] = command;
	}

	/// <summary>嘗試執行命令。</summary>
	public bool TryExecute(string commandId)
	{
		if (!_commands.TryGetValue(commandId, out var cmd))
		{
			return false;
		}

		return cmd.Execute();
	}

	/// <summary>是否已註冊指定命令。</summary>
	public bool Contains(string commandId) => _commands.ContainsKey(commandId);
}
