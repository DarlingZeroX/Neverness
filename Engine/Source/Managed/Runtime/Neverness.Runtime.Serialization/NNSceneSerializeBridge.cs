using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;
using Neverness.Runtime.Scene;

namespace Neverness.Runtime.Serialization;

/// <summary>
/// 场景序列化 ABI 薄封装；经 VFS 路径读写 VGSC 二进制格式。
/// 方法签名对齐新 ABI（layoutVersion = 6）：序列化/反序列化均通过 VFS 虚拟路径。
/// </summary>
public static unsafe class NNSceneSerializeBridge
{
	/// <summary>当前托管侧期望的场景 blob 格式版本（对齐 VGSC 格式版本 2）。</summary>
	public const int ExpectedBlobFormatVersion = 2;

	/// <summary>Scene 子表是否已安装且提供序列化函数指针。</summary>
	public static bool IsAvailable =>
		EngineNativeApiBootstrap.IsInstalled &&
		EngineNativeApiBootstrap.EngineApi.Scene.SerializeScene != null;

	/// <summary>序列化场景并写入 VFS 路径（VGSC 二进制格式）。</summary>
	/// <param name="sceneHandle">场景句柄。</param>
	/// <param name="vfsPath">VFS 虚拟路径（如 "/assets/MyScene.scene"）。</param>
	/// <returns>操作结果。</returns>
	public static NNSceneResult SerializeScene(ulong sceneHandle, string vfsPath)
	{
		return SceneNativeBridge.SerializeScene(sceneHandle, vfsPath);
	}

	/// <summary>自 VFS 路径反序列化，创建新场景。</summary>
	/// <param name="vfsPath">VFS 虚拟路径。</param>
	/// <returns>操作结果和新场景句柄（失败时为 0）。</returns>
	public static (NNSceneResult Result, ulong SceneHandle) DeserializeScene(string vfsPath)
	{
		var result = SceneNativeBridge.DeserializeScene(out var handle, vfsPath);
		return (result, handle);
	}
}
