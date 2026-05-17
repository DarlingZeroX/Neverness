using VisionGal.Managed.Object;

namespace VisionGal.Managed.Scene;

/// <summary>
/// Prefab 描述：可從場景實體範本實例化新 <see cref="SceneEntity"/>。
/// </summary>
public sealed class Prefab
{
	/// <summary>Prefab 名稱。</summary>
	public string Name { get; }

	/// <summary>範本顯示名稱。</summary>
	public string TemplateDisplayName { get; }

	/// <summary>建立 Prefab 描述。</summary>
	public Prefab(string name, string templateDisplayName)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(name);
		TemplateDisplayName = templateDisplayName;
		Name = name;
	}

	/// <summary>實例化新場景實體並註冊。</summary>
	public SceneEntity Instantiate()
	{
		var entity = LifetimeSystem.CreateAndRegister<SceneEntity>("SceneEntity");
		entity.DisplayName = TemplateDisplayName;
		return entity;
	}
}
