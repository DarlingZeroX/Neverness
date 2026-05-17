using VisionGal.Managed.Reflection;

namespace VisionGal.Managed.Inspector;

/// <summary>
/// 屬性繪製器：依 <see cref="PropertyMetadata"/> 讀寫目標成員（供 Editor UI 或測試驅動）。
/// </summary>
public static class PropertyDrawer
{
	/// <summary>讀取屬性目前值。</summary>
	public static object? Read(PropertyMetadata property, object target)
	{
		ArgumentNullException.ThrowIfNull(property);
		ArgumentNullException.ThrowIfNull(target);
		return property.GetValue(target);
	}

	/// <summary>寫入屬性值；若型別不相容則回傳 false。</summary>
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
