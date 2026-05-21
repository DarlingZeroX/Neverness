using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景实体薄门面：持有 Native <see cref="NNEntityHandle"/>，属性经 <see cref="SceneNativeBridge"/> 读写。
/// </summary>
public sealed class SceneEntity
{
	/// <summary>Native 场景图实体句柄。</summary>
	public NNEntityHandle Handle { get; private set; }

	/// <summary>实体显示名称（写入时同步至 Native）。</summary>
	public string DisplayName
	{
		get => _displayName;
		set
		{
			_displayName = value ?? string.Empty;
			if (IsAlive)
			{
				SceneNativeBridge.SetEntityName(Handle, _displayName);
			}
		}
	}

	private string _displayName;

	/// <summary>句柄非零时视为可参与场景 API 操作。</summary>
	public bool IsAlive => SceneNativeBridge.IsEntityAlive(Handle);

	/// <summary>由已有 Native 句柄建立门面（不调用 spawn）。</summary>
	public SceneEntity(NNEntityHandle handle, string displayName = "Entity")
	{
		Handle = handle;
		_displayName = displayName ?? string.Empty;
	}

	/// <summary>经 Native spawn 建立实体并设置显示名。</summary>
	/// <param name="prefabVirtualPathUtf8">Prefab 虚拟路径，传 null 时使用 <c>SceneEntity</c>。</param>
	public static SceneEntity? Spawn(string? prefabVirtualPathUtf8 = null, string displayName = "Entity")
	{
		var path = string.IsNullOrWhiteSpace(prefabVirtualPathUtf8) ? "SceneEntity" : prefabVirtualPathUtf8;
		var handle = SceneNativeBridge.Spawn(path);
		if (handle.Value == 0)
		{
			return null;
		}

		var entity = new SceneEntity(handle, displayName);
		SceneNativeBridge.SetEntityName(handle, entity._displayName);
		return entity;
	}

	/// <summary>销毁 Native 实体并使句柄失效。</summary>
	public void Destroy()
	{
		if (!IsAlive)
		{
			return;
		}

		SceneNativeBridge.Destroy(Handle);
		Handle = default;
	}
}
