using System.Runtime.InteropServices;
using VisionGal.Managed.Engine;

namespace VisionGal.Managed.Assets;

/// <summary>
/// 128-bit 資產 GUID；與 Native <see cref="VGGuid"/> 記憶體佈局一致，可零拷貝互操作。
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public readonly struct GUID : IEquatable<GUID>
{
	/// <summary>高 64 位。</summary>
	public ulong High { get; }

	/// <summary>低 64 位。</summary>
	public ulong Low { get; }

	/// <summary>全零 GUID。</summary>
	public static GUID Zero => new(0, 0);

	/// <summary>是否為全零。</summary>
	public bool IsZero => High == 0 && Low == 0;

	/// <summary>建立 GUID。</summary>
	public GUID(ulong high, ulong low)
	{
		High = high;
		Low = low;
	}

	/// <summary>自 Native <see cref="VGGuid"/> 轉換。</summary>
	public static GUID FromNative(VGGuid native) => new(native.High, native.Low);

	/// <summary>轉為 Native <see cref="VGGuid"/>。</summary>
	public VGGuid ToNative() => new() { High = High, Low = Low };

	/// <summary>
	/// 依虛擬路徑產生穩定、非零之合成 GUID（Native 匯入不可用時之託管回退）。
	/// 使用 FNV-1a 64-bit 雙段雜湊，相同路徑在行程內可重現。
	/// </summary>
	/// <param name="virtualPathUtf8">資產虛擬路徑（UTF-8 語意字串）。</param>
	public static GUID FromDeterministicPath(string virtualPathUtf8)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(virtualPathUtf8);
		var bytes = System.Text.Encoding.UTF8.GetBytes(virtualPathUtf8);
		var high = Fnv1a64(bytes, seed: 0xcbf29ce484222325UL);
		var low = Fnv1a64(bytes, seed: 0x100000001b3UL);
		if (high == 0 && low == 0)
		{
			low = 1;
		}

		return new GUID(high, low);
	}

	private static ulong Fnv1a64(ReadOnlySpan<byte> data, ulong seed)
	{
		const ulong prime = 0x00000100000001B3UL;
		var hash = seed;
		foreach (var b in data)
		{
			hash ^= b;
			hash *= prime;
		}

		return hash;
	}

	/// <summary>自 32 字元十六進位字串解析（可含連字號，忽略非 hex 字元）。</summary>
	public static GUID Parse(string hex)
	{
		ArgumentNullException.ThrowIfNull(hex);
		var digits = new string(hex.Where(Uri.IsHexDigit).ToArray());
		if (digits.Length < 32)
		{
			digits = digits.PadRight(32, '0');
		}

		var high = Convert.ToUInt64(digits[..16], 16);
		var low = Convert.ToUInt64(digits[16..32], 16);
		return new GUID(high, low);
	}

	/// <summary>轉為 32 字元小寫十六進位（無連字號）。</summary>
	public string ToHexString() => $"{High:x16}{Low:x16}";

	/// <inheritdoc />
	public bool Equals(GUID other) => High == other.High && Low == other.Low;

	/// <inheritdoc />
	public override bool Equals(object? obj) => obj is GUID g && Equals(g);

	/// <inheritdoc />
	public override int GetHashCode() => HashCode.Combine(High, Low);

	/// <inheritdoc />
	public override string ToString() => ToHexString();

	public static bool operator ==(GUID left, GUID right) => left.Equals(right);
	public static bool operator !=(GUID left, GUID right) => !left.Equals(right);
}
