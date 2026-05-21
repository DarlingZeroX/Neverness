using Neverness.Editor.Framework.Reflection;

namespace Neverness.Editor.Framework.Inspector;

/// <summary>
/// 属性绘制器：依 <see cref="PropertyMetadata"/> 读写目标成员（供 Editor UI 或测试驱动）。
/// </summary>
public static class PropertyDrawer
{
	/// <summary>读取属性目前值。</summary>
	public static object? Read(PropertyMetadata property, object target)
	{
		ArgumentNullException.ThrowIfNull(property);
		ArgumentNullException.ThrowIfNull(target);
		return property.GetValue(target);
	}

	/// <summary>写入属性值；若类型不兼容则返回 false。</summary>
	public static bool TryWrite(PropertyMetadata property, object target, object? value)
	{
		ArgumentNullException.ThrowIfNull(property);
		ArgumentNullException.ThrowIfNull(target);

		if (value != null && !property.PropertyType.IsInstanceOfType(value))
		{
			try
			{
				value = Convert.ChangeType(value, property.PropertyType);
			}
			catch
			{
				return false;
			}
		}

		if (property.Range is { } range && value is IConvertible c)
		{
			var f = Convert.ToSingle(c);
			if (f < range.Min || f > range.Max)
			{
				return false;
			}
		}

		property.SetValue(target, value);
		return true;
	}
}
