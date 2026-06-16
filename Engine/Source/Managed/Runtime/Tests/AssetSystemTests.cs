using Neverness.Runtime.Assets;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>资产系统扩展单元测试（GUID + AssetHandle + AssetManager）。</summary>
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
		var handle = default(AssetHandle);
		Assert.True(handle.IsZero);
		Assert.Equal(0ul, handle.Value);
	}

	[Fact]
	public void AssetHandle_Zero_IsInvalid()
	{
		Assert.True(AssetHandle.Zero.IsZero);
		Assert.True(AssetHandle<int>.Zero.IsZero);
	}

	[Fact]
	public void AssetHandle_FromRaw_RoundTrip()
	{
		var handle = AssetHandle.FromRaw(0x00000002_00000001ul);
		Assert.False(handle.IsZero);
		Assert.Equal(0x00000002_00000001ul, handle.Value);
	}

	/* ===== AssetManager 测试 ===== */

	[Fact]
	public void AssetManager_Instance_NotNull()
	{
		Assert.NotNull(AssetManager.Instance);
	}

	[Fact]
	public void AssetManager_LoadAssetSync_ZeroGuid_ReturnsZero()
	{
		var handle = AssetManager.Instance.LoadAssetSync(GUID.Zero);
		Assert.Equal(0ul, handle);
	}
}
