namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景门面：运行时经 <see cref="SceneNativeBridge"/> 访问 Native 场景图。
/// </summary>
public sealed class Scene
{
	/// <summary>场景名称。</summary>
	public string Name { get; set; }

	/// <summary>本 Managed 会话跟踪的实体（Native 为权威存储）。</summary>
	public List<SceneEntity> Entities { get; } = [];

	/// <summary>建立场景。</summary>
	public Scene(string name)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		Name = name;
	}

	/// <summary>经 Native <c>loadScene</c> 加载本场景名。</summary>
	public int LoadNative() => SceneNativeBridge.LoadScene(Name);

	/// <summary>经 Native <c>unloadScene</c> 卸载本场景名。</summary>
	public int UnloadNative() => SceneNativeBridge.UnloadScene(Name);

	/// <summary>经 Native spawn 生成实体并加入跟踪列表。</summary>
	public SceneEntity? SpawnEntity(string prefabVirtualPath, string displayName = "Entity")
	{
		var entity = SceneEntity.Spawn(prefabVirtualPath, displayName);
		if (entity != null)
		{
			Entities.Add(entity);
		}

		return entity;
	}

	/// <summary>登记已存在的实体门面。</summary>
	public void AddEntity(SceneEntity entity)
	{
		ArgumentNullException.ThrowIfNull(entity);
		Entities.Add(entity);
	}
}
