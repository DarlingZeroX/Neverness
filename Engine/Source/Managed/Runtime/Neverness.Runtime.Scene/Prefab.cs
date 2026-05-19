namespace Neverness.Managed.Scene;

/// <summary>
/// Prefab 描述：经 Native <see cref="SceneNativeBridge.Spawn"/> 实例化 <see cref="SceneEntity"/>。
/// </summary>
public sealed class Prefab
{
	/// <summary>Prefab 名称。</summary>
	public string Name { get; }

	/// <summary>Native Prefab 虚拟路径。</summary>
	public string VirtualPath { get; }

	/// <summary>模板显示名称。</summary>
	public string TemplateDisplayName { get; }

	/// <summary>建立 Prefab 描述。</summary>
	public Prefab(string name, string virtualPath, string templateDisplayName)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		ArgumentException.ThrowIfNullOrWhiteSpace(virtualPath);
		Name = name;
		VirtualPath = virtualPath;
		TemplateDisplayName = templateDisplayName;
	}

	/// <summary>经 Native spawn 实例化场景实体。</summary>
	public SceneEntity? Instantiate() => SceneEntity.Spawn(VirtualPath, TemplateDisplayName);
}
