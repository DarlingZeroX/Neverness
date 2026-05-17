using Neverness.Managed.Engine;
using Neverness.Managed.Object;
using Neverness.Managed.Reflection;

namespace Neverness.Managed.Scene;

/// <summary>
/// 場景實體：託管 <see cref="VGObject"/> 衍生，具可序列化顯示名稱。
/// </summary>
public sealed class SceneEntity : VGObject
{
	/// <summary>實體顯示名稱。</summary>
	[SerializeField]
	public string DisplayName { get; set; }

	/// <inheritdoc />
	public override string TypeName => "SceneEntity";

	/// <summary>建立場景實體。</summary>
	public SceneEntity(VGObjectId id, VGObjectHandle handle, string displayName = "Entity")
		: base(id, handle)
	{
		DisplayName = displayName;
	}
}
