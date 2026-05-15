using VisionGal.Managed.Object;
using VisionGal.Managed.Serialization;

namespace VisionGal.Managed.Scene;

/// <summary>
/// 場景 DTO → 託管實體之再水合：經 <see cref="LifetimeSystem"/> 建立新 Native 控制代碼並套用序列化屬性。
/// <para>不還原舊 Handle；不呼叫 Native <c>VGSceneAPI</c>（spawn / setParent）。</para>
/// </summary>
public static class SceneRehydrator
{
	/// <summary>
	/// 自 JSON 還原完整場景（容器 + 實體清單）。
	/// </summary>
	/// <param name="json">由 <see cref="Scene.ToJson"/> 產生之 JSON。</param>
	/// <returns>含實體之場景；JSON 無效時回傳 null。</returns>
	public static Scene? RestoreFromJsonWithEntities(string json)
	{
		var doc = SceneSerializer.Deserialize(json);
		return doc == null ? null : RestoreFromDocumentWithEntities(doc);
	}

	/// <summary>
	/// 自場景描述文件還原完整場景：為每個 <see cref="SceneSerializer.SceneEntityEntry"/> 建立 <see cref="VGObject"/> 並註冊。
	/// </summary>
	/// <param name="document">已反序列化之場景 DTO。</param>
	/// <returns>含實體之場景實例。</returns>
	public static Scene RestoreFromDocumentWithEntities(SceneSerializer.SceneDocument document)
	{
		ArgumentNullException.ThrowIfNull(document);

		var scene = Scene.RestoreFromDocument(document);
		foreach (var entry in document.Entities)
		{
			var entity = CreateEntityFromEntry(entry);
			scene.AddEntity(entity);
		}

		return scene;
	}

	/// <summary>
	/// 依條目型別名建立並註冊單一場景實體，再套用屬性 payload。
	/// </summary>
	private static SceneEntity CreateEntityFromEntry(SceneSerializer.SceneEntityEntry entry)
	{
		ArgumentNullException.ThrowIfNull(entry);

		var clrType = ResolveEntityType(entry.TypeName);
		if (!typeof(VGObject).IsAssignableFrom(clrType))
		{
			throw new NotSupportedException($"無法再水合非 VGObject 型別：{clrType.FullName}");
		}

		var nativeTypeName = clrType.Name;
		var entity = CreateAndRegisterEntity(clrType, nativeTypeName);
		SceneSerializer.ApplyEntryProperties(entity, entry);
		return entity;
	}

	/// <summary>解析條目中的 CLR 型別；失敗時回退 <see cref="SceneEntity"/>。</summary>
	private static Type ResolveEntityType(string typeName)
	{
		if (!string.IsNullOrWhiteSpace(typeName))
		{
			var resolved = Type.GetType(typeName, throwOnError: false);
			if (resolved != null)
			{
				return resolved;
			}
		}

		return typeof(SceneEntity);
	}

	/// <summary>
	/// 經 <see cref="LifetimeSystem.CreateAndRegister{T}"/> 建立實體；目前僅支援 <see cref="SceneEntity"/> 或具相容建構子之衍生型別。
	/// </summary>
	/// <param name="nativeTypeName">傳予 Native <c>createObject</c> 之型別識別字串（使用 CLR 型別短名，與 Bootstrap 一致）。</param>
	private static SceneEntity CreateAndRegisterEntity(Type clrType, string nativeTypeName)
	{
		if (clrType == typeof(SceneEntity))
		{
			return LifetimeSystem.CreateAndRegister<SceneEntity>(nativeTypeName);
		}

		if (typeof(SceneEntity).IsAssignableFrom(clrType))
		{
			var method = typeof(LifetimeSystem)
				.GetMethod(nameof(LifetimeSystem.CreateAndRegister), [typeof(string)])!
				.MakeGenericMethod(clrType);
			return (SceneEntity)method.Invoke(null, [nativeTypeName])!;
		}

		throw new NotSupportedException($"再水合僅支援 SceneEntity 衍生型別，收到：{clrType.FullName}");
	}
}
