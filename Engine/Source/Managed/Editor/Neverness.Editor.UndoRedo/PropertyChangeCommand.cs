using System.Reflection;

namespace Neverness.Editor.Framework.UndoRedo;

/// <summary>
/// 修改单一字段或属性之可撤销命令（使用 BCL 反射，仅依赖 Editor 模块）。
/// </summary>
public sealed class PropertyChangeCommand : IUndoableCommand
{
	private readonly object _target;
	private readonly MemberInfo _member;
	private readonly object? _oldValue;
	private readonly object? _newValue;

	/// <inheritdoc />
	public string Description { get; }

	/// <summary>建立属性变更命令（尚未套用新值）。</summary>
	public PropertyChangeCommand(object target, MemberInfo member, object? newValue, string? description = null)
	{
		ArgumentNullException.ThrowIfNull(target);
		ArgumentNullException.ThrowIfNull(member);
		_target = target;
		_member = member;
		_oldValue = ReadMember(member, target);
		_newValue = newValue;
		Description = description ?? $"变更 {member.Name}";
	}

	/// <inheritdoc />
	public void Execute() => WriteMember(_member, _target, _newValue);

	/// <inheritdoc />
	public void Undo() => WriteMember(_member, _target, _oldValue);

	private static object? ReadMember(MemberInfo member, object target) =>
		member switch
		{
			FieldInfo f => f.GetValue(target),
			PropertyInfo p when p.CanRead => p.GetValue(target),
			_ => null
		};

	private static void WriteMember(MemberInfo member, object target, object? value)
	{
		switch (member)
		{
			case FieldInfo f:
				f.SetValue(target, value);
				break;
			case PropertyInfo p when p.CanWrite:
				p.SetValue(target, value);
				break;
		}
	}
}
