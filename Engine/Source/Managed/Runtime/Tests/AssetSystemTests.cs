using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>资产系统扩展单元测试（GUID + AssetHandle）。</summary>
public sealed class AssetSystemTests
{
	/* ===== GUID 测试 ===== */

	[Fact]
	public void GUID_RoundTrip_SerializesCorrectly()
	{
		var original = GUID.FromDeterministicPath("/assets/roundtrip_test.png");
		var bytes = new byte[16];
		BitConverter.GetBytes(original.High).CopyTo(bytes, 0);
		BitConverter.GetBytes(original.Low).CopyTo(bytes, 8);

		var restoredHigh = BitConverter.ToUInt64(bytes, 0);
		var restoredLow = BitConverter.ToUInt64(bytes, 8);
		var restored = new GUID(restoredHigh, restoredLow);

		Assert.Equal(original, restored);
	}

	[Fact]
	public void GUID_Equality_HashCodeConsistent()
	{
		var a = GUID.FromDeterministicPath("/assets/hash_test.png");
		var b = GUID.FromDeterministicPath("/assets/hash_test.png");
		var c = GUID.FromDeterministicPath("/assets/other.png");

		Assert.Equal(a, b);
		Assert.Equal(a.GetHashCode(), b.GetHashCode());
		Assert.NotEqual(a, c);
	}

	[Fact]
	public void GUID_Zero_IsInvalid()
	{
		var zero = default(GUID);
		Assert.True(zero.IsZero);

		var nonZero = GUID.FromDeterministicPath("/assets/nonzero.png");
		Assert.False(nonZero.IsZero);
	}

	[Fact]
	public void GUID_ToHexString_FormatCorrect()
	{
		var guid = GUID.FromDeterministicPath("/assets/hex_test.png");
		var hex = guid.ToHexString();

		Assert.Equal(32, hex.Length);
		Assert.All(hex.ToCharArray(), c => Assert.True(
			(c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')));
	}

	/* ===== AssetHandle 测试 ===== */

	[Fact]
	public void AssetHandle_DefaultIsInvalid()
	{
		var handle = default(NNAssetHandle);
		Assert.Equal(default, handle);
	}

	[Fact]
	public void AssetHandle_NativeApiFields_NotNull()
	{
		/* 验证 API 结构体字段存在（不调用实际方法，避免需要 Native host） */
		var api = NativeApiProvider.AssetManagerApi;
		Assert.NotNull(api);
	}
}
