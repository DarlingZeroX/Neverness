namespace Neverness.Managed.Object;

/// <summary>
/// 託管物件之全域唯一識別碼（64-bit 不透明整數）。
/// 與 Native <c>NNObjectHandle</c> 分離：本型別僅用於託管註冊表索引與序列化引用。
/// </summary>
/// <param name="Value">非零表示有效；零為 <see cref="Invalid"/>。</param>
public readonly record struct VGObjectId(ulong Value)
{
	/// <summary>無效或尚未分配之識別碼。</summary>
	public static VGObjectId Invalid => new(0);

	/// <summary>是否為已分配之非零識別碼。</summary>
	public bool IsValid => Value != 0;

	/// <inheritdoc />
	public override string ToString() => IsValid ? $"VGObjectId({Value})" : "VGObjectId(Invalid)";
}
