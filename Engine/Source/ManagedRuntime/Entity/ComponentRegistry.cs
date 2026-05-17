using System.Collections.Concurrent;

namespace VisionGal.Managed.Entity;

/// <summary>
/// 元件型別全域工廠表：以 <see cref="Type"/> 為鍵登記 <c>EntityHandle → VGComponent</c> 委派，供資料驅動或編輯器匯出層延遲建立元件。
/// </summary>
/// <remarks>
/// <para>實作為 <see cref="System.Collections.Concurrent.ConcurrentDictionary{TKey,TValue}"/>，適合多執行緒註冊；但與 <see cref="EntityWorld"/> 無強耦合，測試時請搭配 <see cref="ClearForTesting"/> 避免汙染他測。</para>
/// <para><b>注意</b>：此為行程級靜態狀態，非場景隔離；正式執行期應由宿主或模組初始化統一註冊，避免重複覆寫導致行為漂移。</para>
/// </remarks>
public static class ComponentRegistry
{
	private static readonly ConcurrentDictionary<Type, Func<EntityHandle, VGComponent>> Factories = new();

	/// <summary>註冊或覆寫某元件型別之工廠委派（以 <c>typeof(T)</c> 為鍵）。</summary>
	public static void Register<T>(Func<EntityHandle, T> factory)
		where T : VGComponent
	{
		ArgumentNullException.ThrowIfNull(factory);
		Factories[typeof(T)] = e => factory(e);
	}

	/// <summary>若 <paramref name="componentType"/> 已註冊工廠則建立實例，否則回傳 null（不拋例外）。</summary>
	public static VGComponent? TryCreate(EntityHandle entity, Type componentType)
	{
		ArgumentNullException.ThrowIfNull(componentType);
		return Factories.TryGetValue(componentType, out var f) ? f(entity) : null;
	}

	/// <summary>清空所有已登記工廠；僅供單元測試或模組卸載，正式遊戲迴圈慎用。</summary>
	public static void ClearForTesting() => Factories.Clear();
}
