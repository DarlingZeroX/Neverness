using System.Text;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Scene;

namespace Neverness.Runtime.Serialization;

/// <summary>
/// 场景序列化桥接——经 VFS 路径读写场景数据。
/// 使用新的 SceneWorld 序列化 API，不再依赖 C++ Native 桥接。
/// </summary>
public static class NNSceneSerializeBridge
{
	/// <summary>当前托管侧期望的场景 blob 格式版本。</summary>
	public const int ExpectedBlobFormatVersion = 2;

	/// <summary>VFS 是否可用。</summary>
	public static bool IsAvailable => Neverness.Runtime.VFS.Public.VFS.IsAvailable;

	/// <summary>序列化场景并写入 VFS 路径（JSON 格式）。</summary>
	/// <param name="world">场景世界。</param>
	/// <param name="vfsPath">VFS 虚拟路径（如 "/assets/MyScene.scene"）。</param>
	/// <returns>操作结果。</returns>
	public static Neverness.Runtime.Scene.NNSceneResult SerializeScene(SceneWorld world, string vfsPath)
	{
		ArgumentNullException.ThrowIfNull(world);
		ArgumentException.ThrowIfNullOrWhiteSpace(vfsPath);

		return world.Save(vfsPath);
	}

	/// <summary>自 VFS 路径反序列化，创建新场景。</summary>
	/// <param name="name">场景名称。</param>
	/// <param name="vfsPath">VFS 虚拟路径。</param>
	/// <returns>场景世界（失败时为 null）。</returns>
	public static SceneWorld? DeserializeScene(string name, string vfsPath)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		ArgumentException.ThrowIfNullOrWhiteSpace(vfsPath);

		return SceneWorld.LoadFromAsset(name, vfsPath);
	}

	/// <summary>序列化场景到字节数组。</summary>
	/// <param name="world">场景世界。</param>
	/// <returns>序列化后的字节数组。</returns>
	public static byte[] SerializeToBytes(SceneWorld world)
	{
		ArgumentNullException.ThrowIfNull(world);

		using var stream = new MemoryStream();
		world.Scene.Serialize(stream, "json");
		return stream.ToArray();
	}

	/// <summary>从字节数组反序列化场景。</summary>
	/// <param name="name">场景名称。</param>
	/// <param name="data">序列化数据。</param>
	/// <returns>场景世界（失败时为 null）。</returns>
	public static SceneWorld? DeserializeFromBytes(string name, byte[] data)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		ArgumentNullException.ThrowIfNull(data);

		var world = SceneWorld.Create(name);
		if (world == null) return null;

		try
		{
			using var stream = new MemoryStream(data);
			world.Scene.Deserialize(stream, "json");
			return world;
		}
		catch
		{
			world.Dispose();
			return null;
		}
	}
}
