using System.Diagnostics.CodeAnalysis;

namespace Neverness.Managed.Entity;

/// <summary>
/// 託管實體世界：負責分配 <see cref="EntityHandle"/>、追蹤存活狀態，並以「每實體一張元件表」掛載 <see cref="VGComponent"/>（首包為 AoS-on-entity，非 Archetype 批次）。
/// </summary>
/// <remarks>
/// <para><b>首包範圍</b>：單執行緒假設、無 SoA／Chunks；與 <c>Neverness.Managed.Scene</c> 之 <b>SceneEntity</b> 並存，不自動同步。Native <b>VGEntityAPI</b>（MANAGED 總覽 <b>§2.7.1</b>）含魔數與 <b>getRuntimeTick</b> 等可觀測項，<b>不</b>表示本類別已與 Kernel 資料鏡像或自動灌入。</para>
/// <para><b>槽位回收</b>：本類別目前<b>不</b>重用已銷毀實體之 Index（內部索引分配器單調遞增），以降低除錯難度；未來若引入自由列表，須維持 Generation 語意不變。</para>
/// <para><b>查詢 API</b>：泛型 <see cref="HasComponent{T}"/>／<see cref="GetComponent{T}"/>／<see cref="TryGetComponent{T}"/> 與非泛型 <see cref="HasComponent(EntityHandle, Type)"/>／<see cref="TryGetComponent(EntityHandle, Type, out VGComponent)"/>／<see cref="GetComponent(EntityHandle, Type)"/>／<see cref="RemoveComponent(EntityHandle, Type)"/> 皆以字典之 <b>精確</b> <see cref="Type"/> 為鍵（與 <see cref="ComponentRegistry.TryCreate"/> 所傳之具體元件型別一致；**Type** 鍵重載與泛型 API 成對，供資料驅動／反射路徑）；另見 <see cref="GetComponentCount"/>。仍屬單執行緒首包；與 Native <b>VGEntityHandle</b>（見 <c>VGNativeEngineAPI/SceneAPI.h</c>）語意獨立，無自動映射或同步。</para>
/// </remarks>
public sealed class EntityWorld
{
	// 下一個可用的實體槽位 Index（自 1 起）；銷毀實體後本首包不回收 Index，故僅遞增。
	private uint _nextIndex = 1;

	/// <summary>單一實體之內部狀態：世代、存活旗標、以及以 CLR <see cref="Type"/> 為鍵的元件字典。</summary>
	private sealed class EntityRecord
	{
		/// <summary>對外 <see cref="EntityHandle.Generation"/> 來源；Spawn 時從 1 開始。</summary>
		public uint Generation = 1;

		/// <summary>為 false 表示已 <see cref="EntityWorld.Destroy"/>，舊 Handle 之 Generation 比對將失敗。</summary>
		public bool Alive = true;

		/// <summary>每型別至多一個元件實例（以 <see cref="VGComponent"/> 衍生型別註冊）；替換同型別走 <see cref="AddComponent{T}"/>。</summary>
		public readonly Dictionary<Type, VGComponent> Components = new();
	}

	// Index → 內部記錄；已銷毀之槽位仍保留記錄以便 Generation 遞增後與舊 Handle 區分（本首包不重用 Index）。
	private readonly Dictionary<uint, EntityRecord> _records = new();

	/// <summary>配置新實體槽位並回傳對應 <see cref="EntityHandle"/>（Index 遞增、Generation 為當前記錄值）。</summary>
	public EntityHandle Spawn()
	{
		// 分配新 Index；本首包不回收已銷毀槽位之 Index（見類 remarks「槽位回收」）。
		var index = _nextIndex++;
		var record = new EntityRecord();
		_records[index] = record;
		return new EntityHandle { Index = index, Generation = record.Generation };
	}

	/// <summary>
	/// 銷毀實體：清空元件表、標記死亡並遞增內部 <see cref="EntityRecord.Generation"/>，使持有舊代碼之呼叫端無法再通過 <see cref="IsAlive"/> 或元件存取。
	/// </summary>
	/// <param name="handle">欲銷毀之代碼；若已無效、已死亡或世代不符則靜默返回（冪等、防雙重 Destroy）。</param>
	public void Destroy(EntityHandle handle)
	{
		// 無效代碼、無記錄、或已標記死亡：直接返回（冪等，避免重複清空元件表）。
		if (!handle.IsValid || !_records.TryGetValue(handle.Index, out var rec) || !rec.Alive)
		{
			return;
		}

		// 懸掛引用：呼叫端持有的是舊世代 Handle，與當前槽位狀態不一致，拒絕操作以免誤傷新實體。
		if (rec.Generation != handle.Generation)
		{
			return;
		}

		rec.Components.Clear();
		rec.Alive = false;
		rec.Generation++; // 使舊 EntityHandle 永久失效；若未來重用 Index，新 Spawn 須使用遞增後之 Generation。
	}

	/// <summary>當 Handle 有效、記錄存在、標記存活且世代一致時回傳 true。</summary>
	public bool IsAlive(EntityHandle handle)
	{
		if (!handle.IsValid)
		{
			return false;
		}

		// 須同時滿足：有記錄、仍標記存活、且對外 Generation 與 Handle 一致（防懸掛引用）。
		return _records.TryGetValue(handle.Index, out var rec) && rec.Alive && rec.Generation == handle.Generation;
	}

	/// <summary>
	/// 掛載或替換指定型別之元件：以 <c>typeof(T)</c> 為鍵；若已存在同鍵實例則覆寫。
	/// </summary>
	/// <typeparam name="T">必須為 <see cref="VGComponent"/> 衍生型別。</typeparam>
	/// <param name="handle">目標實體，須通過 <see cref="IsAlive"/>。</param>
	/// <param name="component">非 null；其建構時綁定之 <see cref="VGComponent.Entity"/> 應與 <paramref name="handle"/> 一致（本類別不強制校驗，由呼叫端約定）。</param>
	/// <exception cref="InvalidOperationException">實體已死亡或 Handle 世代不符。</exception>
	public void AddComponent<T>(EntityHandle handle, T component)
		where T : VGComponent
	{
		ArgumentNullException.ThrowIfNull(component);
		if (!IsAlive(handle))
		{
			throw new InvalidOperationException("實體無效或已銷毀，無法掛載元件。");
		}

		var rec = _records[handle.Index];
		// 以衍生型別 Runtime Type 為鍵；同鍵已存在則覆寫（每型別至多一個元件實例）。
		rec.Components[typeof(T)] = component;
	}

	/// <summary>若實體存活且已掛載 <typeparamref name="T"/>，則透過 <paramref name="component"/> 輸出並回傳 true；否則 false。</summary>
	public bool TryGetComponent<T>(EntityHandle handle, [NotNullWhen(true)] out T? component)
		where T : VGComponent
	{
		component = null;
		if (!IsAlive(handle))
		{
			return false;
		}

		if (!_records[handle.Index].Components.TryGetValue(typeof(T), out var c) || c is not T t)
		{
			return false;
		}

		component = t;
		return true;
	}

	/// <summary>
	/// 若實體存活且已掛載 <typeparamref name="T"/> 則回傳 true；實體無效、已銷毀、世代不符或未掛載該型別時回傳 false（不拋例外）。
	/// </summary>
	/// <typeparam name="T">必須為 <see cref="VGComponent"/> 衍生型別。</typeparam>
	/// <param name="handle">欲查詢之實體代碼。</param>
	public bool HasComponent<T>(EntityHandle handle)
		where T : VGComponent
	{
		if (!IsAlive(handle))
		{
			return false;
		}

		return _records[handle.Index].Components.ContainsKey(typeof(T));
	}

	/// <summary>
	/// 取得實體上已掛載之 <typeparamref name="T"/> 元件；呼叫端確信存在時可省去 <see cref="TryGetComponent{T}"/> 分支。
	/// </summary>
	/// <typeparam name="T">必須為 <see cref="VGComponent"/> 衍生型別。</typeparam>
	/// <param name="handle">目標實體，須存活且已掛載該型別元件。</param>
	/// <returns>非 null 之元件實例。</returns>
	/// <exception cref="InvalidOperationException">實體無效、已銷毀、世代不符，或尚未掛載 <typeparamref name="T"/>。</exception>
	public T GetComponent<T>(EntityHandle handle)
		where T : VGComponent
	{
		if (!IsAlive(handle))
		{
			throw new InvalidOperationException("實體無效或已銷毀，無法存取元件。");
		}

		if (!_records[handle.Index].Components.TryGetValue(typeof(T), out var c) || c is not T t)
		{
			throw new InvalidOperationException("實體上未掛載指定型別之元件。");
		}

		return t;
	}

	/// <summary>
	/// 回傳存活實體上當前已掛載之元件個數（以 <see cref="VGComponent"/> 衍生型別種類計，每型別至多一個）；實體無效、已銷毀或世代不符時回傳 0。
	/// </summary>
	/// <param name="handle">欲統計之實體代碼。</param>
	/// <returns>非負整數；僅在 <see cref="IsAlive"/> 為 true 時可能大於 0。</returns>
	/// <remarks>
	/// 供除錯、測試斷言或簡單工具 UI 使用；非 Archetype 批次查詢，亦不暴露內部字典之可變引用。
	/// </remarks>
	public int GetComponentCount(EntityHandle handle)
	{
		if (!IsAlive(handle))
		{
			return 0;
		}

		return _records[handle.Index].Components.Count;
	}

	/// <summary>
	/// 以執行時 <see cref="Type"/> 精確鍵查詢是否已掛載該元件（與 <see cref="ComponentRegistry.TryCreate"/> 使用之型別鍵一致；不支援以基底型別一次匹配多個衍生鍵）。
	/// </summary>
	/// <param name="handle">欲查詢之實體代碼。</param>
	/// <param name="componentType">須為可指派給 <see cref="VGComponent"/> 之型別；不可為 null。</param>
	/// <returns>實體存活且字典含該鍵時為 true；否則 false。</returns>
	/// <exception cref="ArgumentNullException"><paramref name="componentType"/> 為 null。</exception>
	/// <exception cref="ArgumentException"><paramref name="componentType"/> 不可指派給 <see cref="VGComponent"/>。</exception>
	public bool HasComponent(EntityHandle handle, Type componentType)
	{
		ThrowIfInvalidComponentLookupType(componentType);
		if (!IsAlive(handle))
		{
			return false;
		}

		return _records[handle.Index].Components.ContainsKey(componentType);
	}

	/// <summary>
	/// 以執行時 <see cref="Type"/> 精確鍵嘗試取得已掛載之 <see cref="VGComponent"/>（與泛型 <see cref="TryGetComponent{T}"/> 及 <see cref="ComponentRegistry.TryCreate"/> 鍵語意一致）。
	/// </summary>
	/// <param name="handle">目標實體代碼。</param>
	/// <param name="componentType">須為可指派給 <see cref="VGComponent"/> 之型別；不可為 null。</param>
	/// <param name="component">成功時為非 null 之元件實例。</param>
	/// <returns>實體存活且字典命中時為 true。</returns>
	/// <exception cref="ArgumentNullException"><paramref name="componentType"/> 為 null。</exception>
	/// <exception cref="ArgumentException"><paramref name="componentType"/> 不可指派給 <see cref="VGComponent"/>。</exception>
	public bool TryGetComponent(EntityHandle handle, Type componentType, [NotNullWhen(true)] out VGComponent? component)
	{
		ThrowIfInvalidComponentLookupType(componentType);
		component = null;
		if (!IsAlive(handle))
		{
			return false;
		}

		return _records[handle.Index].Components.TryGetValue(componentType, out component);
	}

	/// <summary>
	/// 以執行時 <see cref="Type"/> 精確鍵取得已掛載之 <see cref="VGComponent"/>（與泛型 <see cref="GetComponent{T}"/> 及 <see cref="ComponentRegistry.TryCreate"/> 鍵語意一致）。
	/// </summary>
	/// <param name="handle">目標實體，須存活且已掛載該型別元件。</param>
	/// <param name="componentType">須為可指派給 <see cref="VGComponent"/> 之型別；不可為 null。</param>
	/// <returns>非 null 之元件實例。</returns>
	/// <exception cref="ArgumentNullException"><paramref name="componentType"/> 為 null。</exception>
	/// <exception cref="ArgumentException"><paramref name="componentType"/> 不可指派給 <see cref="VGComponent"/>。</exception>
	/// <exception cref="InvalidOperationException">實體無效、已銷毀、世代不符，或尚未掛載該鍵之元件。</exception>
	/// <remarks>**Type** 鍵重載與泛型 API 成對，供資料驅動／反射路徑。</remarks>
	public VGComponent GetComponent(EntityHandle handle, Type componentType)
	{
		ThrowIfInvalidComponentLookupType(componentType);
		if (!IsAlive(handle))
		{
			throw new InvalidOperationException("實體無效或已銷毀，無法存取元件。");
		}

		if (!_records[handle.Index].Components.TryGetValue(componentType, out var c))
		{
			throw new InvalidOperationException("實體上未掛載指定型別之元件。");
		}

		return c;
	}

	/// <summary>
	/// 以執行時 <see cref="Type"/> 精確鍵自存活實體移除元件（與泛型 <see cref="RemoveComponent{T}"/> 鍵語意一致）；實體已死或無該鍵時靜默返回。
	/// </summary>
	/// <param name="handle">目標實體代碼。</param>
	/// <param name="componentType">須為可指派給 <see cref="VGComponent"/> 之型別；不可為 null。</param>
	/// <exception cref="ArgumentNullException"><paramref name="componentType"/> 為 null。</exception>
	/// <exception cref="ArgumentException"><paramref name="componentType"/> 不可指派給 <see cref="VGComponent"/>。</exception>
	/// <remarks>**Type** 鍵重載與泛型 API 成對，供資料驅動／反射路徑。</remarks>
	public void RemoveComponent(EntityHandle handle, Type componentType)
	{
		ThrowIfInvalidComponentLookupType(componentType);
		if (!IsAlive(handle))
		{
			return;
		}

		_records[handle.Index].Components.Remove(componentType);
	}

	/// <summary>自存活實體移除 <typeparamref name="T"/> 鍵之元件；實體已死或無該元件時靜默返回。</summary>
	public void RemoveComponent<T>(EntityHandle handle)
		where T : VGComponent
	{
		if (!IsAlive(handle))
		{
			return;
		}

		// 僅在仍存活時修改字典；已死實體不重建記錄。
		_records[handle.Index].Components.Remove(typeof(T));
	}

	/// <summary>
	/// 清空所有實體記錄並重置 Index 配置器；僅供單元測試或開發用關卡卸載，生產路徑應改用明確 Destroy 語意。
	/// </summary>
	public void ClearForTesting()
	{
		// 測試隔離：連同已銷毀之記錄一併移除，避免跨用例汙染 Index／Generation。
		_records.Clear();
		_nextIndex = 1;
	}

	/// <summary>校驗非泛型查詢、<see cref="GetComponent(EntityHandle, Type)"/>／<see cref="RemoveComponent(EntityHandle, Type)"/> 及 <see cref="ComponentRegistry"/> 路徑所傳之 <see cref="Type"/>。</summary>
	/// <param name="componentType">呼叫端傳入之元件型別。</param>
	/// <exception cref="ArgumentNullException">型別為 null。</exception>
	/// <exception cref="ArgumentException">型別不可指派給 <see cref="VGComponent"/>。</exception>
	private static void ThrowIfInvalidComponentLookupType(Type componentType)
	{
		ArgumentNullException.ThrowIfNull(componentType);
		if (!componentType.IsAssignableTo(typeof(VGComponent)))
		{
			throw new ArgumentException("型別必須可指派給 VGComponent。", nameof(componentType));
		}
	}
}
