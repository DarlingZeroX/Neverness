// Neverness.Editor.Scene — 经 NNEditorSceneApi 访问 Native Editor 场景快照查询。
// 独立于 SceneNativeBridge（Runtime 用），仅 Editor 模块消费。

using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Editor.Scene.Private;

/// <summary>
/// Editor 专用场景查询桥接——通过 <see cref="NNEditorSceneApi"/> 函数表调用。
/// 独立于 <c>SceneNativeBridge</c>（Runtime 操作），仅 Editor 模块消费。
/// </summary>
internal static unsafe class EditorSceneNativeBridge
{
	/// <summary>Editor Scene 子表是否已安装且可用。</summary>
	public static bool IsAvailable =>
		EngineNativeApiBootstrap.IsInstalled &&
		EngineNativeApiBootstrap.EngineApi.EditorScene.GetHierarchyVersion != null;

	/// <summary>查询 hierarchyVersion（每帧调用，纯整数返回，最轻量 P/Invoke）。</summary>
	public static ulong GetHierarchyVersion(ulong sceneHandle)
	{
		if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetHierarchyVersion == null)
			return 0;
		return api.GetHierarchyVersion(sceneHandle);
	}

	/// <summary>查询 snapshot 所需缓冲区大小（字节）。</summary>
	public static uint GetSnapshotSize(ulong sceneHandle)
	{
		if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetSnapshotSize == null)
			return 0;
		return api.GetSnapshotSize(sceneHandle);
	}

	/// <summary>拷贝完整 snapshot 到调用方缓冲区。返回实际写入字节数。</summary>
	public static uint GetHierarchySnapshot(ulong sceneHandle, void* buffer, uint capacity)
	{
		if (sceneHandle == 0 || buffer == null || !TryGetApi(out var api) || api.GetHierarchySnapshot == null)
			return 0;
		return api.GetHierarchySnapshot(sceneHandle, buffer, capacity);
	}

	/// <summary>查询 transformVersion。</summary>
	public static ulong GetTransformVersion(ulong sceneHandle)
	{
		if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetTransformVersion == null)
			return 0;
		return api.GetTransformVersion(sceneHandle);
	}

	/// <summary>按 Entity 列表批量获取 Transform 数据。返回实际写入元素数。</summary>
	public static uint GetTransformSnapshot(
		ulong sceneHandle,
		ReadOnlySpan<ulong> entities,
		Span<NNEditorTransformData> outData)
	{
		if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetTransformSnapshot == null)
			return 0;

		fixed (ulong* pEntities = entities)
		fixed (NNEditorTransformData* pOut = outData)
		{
			return api.GetTransformSnapshot(sceneHandle, pEntities, (uint)entities.Length, pOut);
		}
	}

	/// <summary>拷贝增量脏条目到调用方缓冲区。返回写入字节数（entryCount * 16）。</summary>
	public static uint GetIncrementalSnapshot(ulong sceneHandle, void* buffer, uint capacity)
	{
		if (sceneHandle == 0 || buffer == null || !TryGetApi(out var api) || api.GetIncrementalSnapshot == null)
			return 0;
		return api.GetIncrementalSnapshot(sceneHandle, buffer, capacity);
	}

	private static bool TryGetApi(out NNEditorSceneApi api)
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			api = default;
			return false;
		}
		api = EngineNativeApiBootstrap.EngineApi.EditorScene;
		return true;
	}
}
