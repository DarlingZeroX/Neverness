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

	// ── Reflection API（layoutVersion = 3）──

	/// <summary>查询 reflection 版本号（组件增删时递增）。C# 每帧 poll。</summary>
	public static ulong GetReflectionVersion(ulong sceneHandle)
	{
		if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetReflectionVersion == null)
			return 0;
		return api.GetReflectionVersion(sceneHandle);
	}

	/// <summary>查询类型信息快照所需缓冲区大小（字节）。</summary>
	public static uint GetTypeInfoSnapshotSize(ulong sceneHandle)
	{
		if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetTypeInfoSnapshotSize == null)
			return 0;
		return api.GetTypeInfoSnapshotSize(sceneHandle);
	}

	/// <summary>拷贝类型信息快照到调用方缓冲区。返回实际写入字节数。</summary>
	public static uint GetTypeInfoSnapshot(ulong sceneHandle, void* buffer, uint capacity)
	{
		if (sceneHandle == 0 || buffer == null || !TryGetApi(out var api) || api.GetTypeInfoSnapshot == null)
			return 0;
		return api.GetTypeInfoSnapshot(sceneHandle, buffer, capacity);
	}

	/// <summary>查询实体拥有的组件数量。</summary>
	public static uint GetEntityComponentCount(ulong sceneHandle, ulong entity)
	{
		if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetEntityComponentCount == null)
			return 0;
		return api.GetEntityComponentCount(sceneHandle, entity);
	}

	/// <summary>拷贝实体的组件信息数组。返回实际写入的条目数。</summary>
	public static uint GetEntityComponents(
		ulong sceneHandle,
		ulong entity,
		Span<NNEditorComponentInfo> outInfos)
	{
		if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetEntityComponents == null)
			return 0;
		fixed (NNEditorComponentInfo* pOut = outInfos)
		{
			return api.GetEntityComponents(sceneHandle, entity, pOut, (uint)outInfos.Length);
		}
	}

	/// <summary>拷贝指定组件类型的字段信息。返回实际写入的条目数。</summary>
	public static uint GetComponentFieldInfos(
		ulong sceneHandle,
		ulong componentTypeId,
		Span<NNEditorFieldInfo> outFields)
	{
		if (sceneHandle == 0 || !TryGetApi(out var api) || api.GetComponentFieldInfos == null)
			return 0;
		fixed (NNEditorFieldInfo* pOut = outFields)
		{
			return api.GetComponentFieldInfos(sceneHandle, componentTypeId, pOut, (uint)outFields.Length);
		}
	}

	/// <summary>拷贝实体的指定组件原始数据到调用方缓冲区。返回实际写入字节数。</summary>
	public static uint GetComponentRawData(
		ulong sceneHandle,
		ulong entity,
		ulong componentTypeId,
		void* outData,
		uint capacity)
	{
		if (sceneHandle == 0 || outData == null || !TryGetApi(out var api) || api.GetComponentRawData == null)
			return 0;
		return api.GetComponentRawData(sceneHandle, entity, componentTypeId, outData, capacity);
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
