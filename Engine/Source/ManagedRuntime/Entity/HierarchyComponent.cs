namespace VisionGal.Managed.Entity;

/// <summary>
/// 父子層級資料：以 <see cref="Parent"/> 指向父實體；根節點使用 <see cref="EntityHandle.Invalid"/>。
/// </summary>
/// <remarks>
/// 僅儲存邊資料，不做循環偵測、不做與 Native Scene Graph 或 <c>SceneEntity</c> 之自動同步（對齊 P1 <b>VGSceneRuntime</b> 後續）。
/// </remarks>
public sealed class HierarchyComponent : VGComponent
{
	/// <inheritdoc />
	public HierarchyComponent(EntityHandle entity)
		: base(entity)
	{
		Parent = EntityHandle.Invalid;
	}

	/// <summary>父實體代碼；無父時為 <see cref="EntityHandle.Invalid"/>。</summary>
	public EntityHandle Parent { get; set; }
}
