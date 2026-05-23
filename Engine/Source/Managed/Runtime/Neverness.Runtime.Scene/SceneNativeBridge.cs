// Neverness.Runtime.Scene — 经 NNSceneAPI 访问 Native ECS 场景；C# SceneManager 驱动生命周期。

using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Runtime.Scene;

/// <summary>
/// <see cref="NNSceneApi"/> 薄封装：Managed 不持有 ECS 存储，仅经 ABI 转发。
/// 方法签名对齐新 ABI（layoutVersion = 4）：统一 <see cref="NNSceneResult"/> 返回值，
/// NNSceneHandle = ulong，componentTypeId = ulong（FNV-1a name hash）。
/// </summary>
public static unsafe class SceneNativeBridge
{
	/// <summary>引擎 Scene 子表是否已安装且可用。</summary>
	public static bool IsAvailable =>
		EngineNativeApiBootstrap.IsInstalled &&
		EngineNativeApiBootstrap.EngineApi.Scene.CreateScene != null;

	/// <summary>句柄是否非零（不保证 Native 槽位仍有效，销毁后勿复用）。</summary>
	public static bool IsEntityAlive(NNEntityHandle handle) => handle.Value != 0;

	// ── 场景管理 ──

	/// <summary>创建空场景；成功时返回 <see cref="NNSceneResult.Ok"/> 并填入场景句柄。</summary>
	public static NNSceneResult CreateScene(out ulong sceneHandle)
	{
		sceneHandle = 0;
		if (!TryGetSceneApi(out var api) || api.CreateScene == null)
		{
			return NNSceneResult.Invalid;
		}

		fixed (ulong* h = &sceneHandle)
		{
			return api.CreateScene(h);
		}
	}

	/// <summary>销毁场景及其所有实体。</summary>
	public static NNSceneResult DestroyScene(ulong sceneHandle)
	{
		if (sceneHandle == 0 || !TryGetSceneApi(out var api) || api.DestroyScene == null)
		{
			return NNSceneResult.Invalid;
		}

		return api.DestroyScene(sceneHandle);
	}

	/// <summary>驱动场景内 ECS System 调度器 Tick。</summary>
	public static NNSceneResult TickSystems(ulong sceneHandle, float deltaTime)
	{
		if (sceneHandle == 0 || !TryGetSceneApi(out var api) || api.TickSystems == null)
		{
			return NNSceneResult.Invalid;
		}

		return api.TickSystems(sceneHandle, deltaTime);
	}

	// ── 实体 CRUD ──

	/// <summary>在场景中创建实体（含默认组件）；失败返回零句柄。</summary>
	public static NNEntityHandle CreateEntity(ulong sceneHandle)
	{
		if (sceneHandle == 0 || !TryGetSceneApi(out var api) || api.CreateEntity == null)
		{
			return default;
		}

		ulong entity = 0;
		api.CreateEntity(sceneHandle, &entity);
		return new NNEntityHandle(entity);
	}

	/// <summary>销毁场景实体。</summary>
	public static NNSceneResult DestroyEntity(ulong sceneHandle, NNEntityHandle entity)
	{
		if (entity.Value == 0 || sceneHandle == 0 || !TryGetSceneApi(out var api) || api.DestroyEntity == null)
		{
			return NNSceneResult.Invalid;
		}

		return api.DestroyEntity(sceneHandle, entity.Value);
	}

	// ── 组件操作（泛型封装）──

	/// <summary>给实体添加组件（通过 <typeparamref name="T"/> 的 FNV-1a name hash 标识）。</summary>
	public static NNSceneResult AddComponent<T>(ulong sceneHandle, NNEntityHandle entity) where T : struct
	{
		if (entity.Value == 0 || sceneHandle == 0 || !TryGetSceneApi(out var api) || api.AddComponent == null)
		{
			return NNSceneResult.Invalid;
		}

		var typeId = ComponentTypeCache<T>.TypeId;
		return api.AddComponent(sceneHandle, entity.Value, typeId);
	}

	/// <summary>移除实体组件。</summary>
	public static NNSceneResult RemoveComponent<T>(ulong sceneHandle, NNEntityHandle entity) where T : struct
	{
		if (entity.Value == 0 || sceneHandle == 0 || !TryGetSceneApi(out var api) || api.RemoveComponent == null)
		{
			return NNSceneResult.Invalid;
		}

		var typeId = ComponentTypeCache<T>.TypeId;
		return api.RemoveComponent(sceneHandle, entity.Value, typeId);
	}

	/// <summary>给实体添加组件（通过原始 TypeId 标识，Prefab / 覆盖使用）。</summary>
	public static NNSceneResult AddComponent(ulong sceneHandle, NNEntityHandle entity, ulong componentTypeId)
	{
		if (entity.Value == 0 || sceneHandle == 0 || !TryGetSceneApi(out var api) || api.AddComponent == null)
		{
			return NNSceneResult.Invalid;
		}

		return api.AddComponent(sceneHandle, entity.Value, componentTypeId);
	}

	/// <summary>移除实体组件（通过原始 TypeId 标识）。</summary>
	public static NNSceneResult RemoveComponent(ulong sceneHandle, NNEntityHandle entity, ulong componentTypeId)
	{
		if (entity.Value == 0 || sceneHandle == 0 || !TryGetSceneApi(out var api) || api.RemoveComponent == null)
		{
			return NNSceneResult.Invalid;
		}

		return api.RemoveComponent(sceneHandle, entity.Value, componentTypeId);
	}

	/// <summary>查询实体是否拥有指定组件。</summary>
	public static bool HasComponent<T>(ulong sceneHandle, NNEntityHandle entity) where T : struct
	{
		if (entity.Value == 0 || sceneHandle == 0 || !TryGetSceneApi(out var api) || api.HasComponent == null)
		{
			return false;
		}

		var typeId = ComponentTypeCache<T>.TypeId;
		int has = 0;
		api.HasComponent(sceneHandle, entity.Value, typeId, &has);
		return has != 0;
	}

	/// <summary>读取组件数据；失败返回 null。</summary>
	public static T? GetComponent<T>(ulong sceneHandle, NNEntityHandle entity) where T : struct
	{
		if (entity.Value == 0 || sceneHandle == 0 || !TryGetSceneApi(out var api) || api.GetComponent == null)
		{
			return null;
		}

		var typeId = ComponentTypeCache<T>.TypeId;
		T value = default;
		var size = (uint)Marshal.SizeOf<T>();
		var result = api.GetComponent(sceneHandle, entity.Value, typeId, &value, size);
		return result == NNSceneResult.Ok ? value : null;
	}

	/// <summary>写入组件数据。</summary>
	public static NNSceneResult SetComponent<T>(ulong sceneHandle, NNEntityHandle entity, T data) where T : struct
	{
		if (entity.Value == 0 || sceneHandle == 0 || !TryGetSceneApi(out var api) || api.SetComponent == null)
		{
			return NNSceneResult.Invalid;
		}

		var typeId = ComponentTypeCache<T>.TypeId;
		var size = (uint)Marshal.SizeOf<T>();
		return api.SetComponent(sceneHandle, entity.Value, typeId, &data, size);
	}

	/// <summary>从原始字节数组写入组件数据（Prefab / 反序列化使用）。</summary>
	public static NNSceneResult SetComponentData(
		ulong sceneHandle, NNEntityHandle entity, ulong componentTypeId, ReadOnlySpan<byte> data)
	{
		if (entity.Value == 0 || sceneHandle == 0 || !TryGetSceneApi(out var api) || api.SetComponent == null)
		{
			return NNSceneResult.Invalid;
		}

		fixed (byte* p = data)
		{
			return api.SetComponent(sceneHandle, entity.Value, componentTypeId, p, (uint)data.Length);
		}
	}

	// ── 层级 ──

	/// <summary>设置父子关系。</summary>
	public static NNSceneResult SetParent(ulong sceneHandle, NNEntityHandle child, NNEntityHandle parent)
	{
		if (child.Value == 0 || sceneHandle == 0 || !TryGetSceneApi(out var api) || api.SetParent == null)
		{
			return NNSceneResult.Invalid;
		}

		return api.SetParent(sceneHandle, child.Value, parent.Value);
	}

	/// <summary>获取父实体；无父时返回零句柄。</summary>
	public static NNEntityHandle GetParent(ulong sceneHandle, NNEntityHandle entity)
	{
		if (entity.Value == 0 || sceneHandle == 0 || !TryGetSceneApi(out var api) || api.GetParent == null)
		{
			return default;
		}

		ulong parent = 0;
		api.GetParent(sceneHandle, entity.Value, &parent);
		return new NNEntityHandle(parent);
	}

	// ── 序列化（经 VFS 路径）──

	/// <summary>序列化场景并写入 VFS 路径（VGSC 二进制格式）。</summary>
	public static NNSceneResult SerializeScene(ulong sceneHandle, string vfsPath)
	{
		if (sceneHandle == 0 || string.IsNullOrEmpty(vfsPath) || !TryGetSceneApi(out var api) || api.SerializeScene == null)
		{
			return NNSceneResult.Invalid;
		}

		var pathBytes = System.Text.Encoding.UTF8.GetBytes(vfsPath + '\0');
		fixed (byte* p = pathBytes)
		{
			return api.SerializeScene(sceneHandle, p);
		}
	}

	/// <summary>自 VFS 路径读取 VGSC 二进制并反序列化，创建新场景。</summary>
	public static NNSceneResult DeserializeScene(out ulong sceneHandle, string vfsPath)
	{
		sceneHandle = 0;
		if (string.IsNullOrEmpty(vfsPath) || !TryGetSceneApi(out var api) || api.DeserializeScene == null)
		{
			return NNSceneResult.Invalid;
		}

		var pathBytes = System.Text.Encoding.UTF8.GetBytes(vfsPath + '\0');
		fixed (byte* p = pathBytes)
		fixed (ulong* h = &sceneHandle)
		{
			return api.DeserializeScene(h, p);
		}
	}

	internal static bool TryGetSceneApi(out NNSceneApi api)
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			api = default;
			return false;
		}

		api = EngineNativeApiBootstrap.EngineApi.Scene;
		return true;
	}
}

/// <summary>
/// 【已废弃】请使用 <see cref="ComponentTypeCache{T}"/>。
/// 该类仅保留向后兼容，后续版本将移除。
/// </summary>
[Obsolete("Use ComponentTypeCache<T> instead. ComponentTypeRegistry<T> will be removed in a future version.")]
internal static class ComponentTypeRegistry<T> where T : struct
{
	/// <summary>FNV-1a 64-bit hash of typeof(T).Name（UTF-8）。</summary>
	public static readonly ulong TypeId = ComputeTypeId();

	private static ulong ComputeTypeId()
	{
		var name = typeof(T).Name;
		ulong hash = 14695981039346656037UL;
		foreach (var c in name)
		{
			hash ^= (byte)c;
			hash *= 1099511628211UL;
		}
		return hash;
	}
}
