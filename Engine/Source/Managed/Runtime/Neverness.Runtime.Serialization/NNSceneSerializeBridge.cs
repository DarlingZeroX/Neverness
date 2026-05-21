using System.Text;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Runtime.Serialization;

/// <summary>
/// 场景二进制快照 ABI 薄封装；经 <see cref="EngineNativeApiBootstrap.EngineApi.Scene"/> 转发至 Native <c>NNSceneSerializer</c>。
/// </summary>
public static unsafe class NNSceneSerializeBridge
{
	/// <summary>当前托管侧期望的场景 blob 格式版本（与 Native 对齐前仅作文档常量）。</summary>
	public const int ExpectedBlobFormatVersion = 1;

	/// <summary>Scene 子表是否已安装且提供序列化函数指针。</summary>
	public static bool IsAvailable =>
		EngineNativeApiBootstrap.IsInstalled &&
		EngineNativeApiBootstrap.EngineApi.Scene.SerializeScene != null;

	/// <summary>序列化命名场景为二进制 blob；失败或容量不足时返回空数组。</summary>
	public static byte[] SerializeScene(string sceneNameUtf8, int initialCapacity = 4096)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(sceneNameUtf8);
		if (!IsAvailable)
		{
			return [];
		}

		var utf8 = Encoding.UTF8.GetBytes(sceneNameUtf8);
		var buffer = new byte[Math.Max(initialCapacity, 256)];
		var serialize = EngineNativeApiBootstrap.EngineApi.Scene.SerializeScene;
		if (serialize == null)
		{
			return [];
		}

		fixed (byte* namePtr = utf8)
		fixed (byte* blobPtr = buffer)
		{
			var written = serialize(namePtr, blobPtr, (nuint)buffer.Length);
			if (written == 0 || written > (nuint)buffer.Length)
			{
				return [];
			}

			var result = new byte[written];
			Buffer.BlockCopy(buffer, 0, result, 0, (int)written);
			return result;
		}
	}

	/// <summary>自二进制 blob 还原场景；ABI 未接线或失败时返回 false。</summary>
	public static bool TryDeserializeScene(ReadOnlySpan<byte> blob, string sceneNameUtf8)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(sceneNameUtf8);
		if (blob.Length == 0 || !EngineNativeApiBootstrap.IsInstalled)
		{
			return false;
		}

		var deserialize = EngineNativeApiBootstrap.EngineApi.Scene.DeserializeScene;
		if (deserialize == null)
		{
			return false;
		}

		var utf8 = Encoding.UTF8.GetBytes(sceneNameUtf8);
		fixed (byte* blobPtr = blob)
		fixed (byte* namePtr = utf8)
		{
			return deserialize(blobPtr, (nuint)blob.Length, namePtr) != 0;
		}
	}
}
