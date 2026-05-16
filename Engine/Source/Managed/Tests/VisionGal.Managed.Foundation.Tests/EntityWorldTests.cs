using VisionGal.Managed.Entity;

namespace VisionGal.Managed.Foundation.Tests;

/// <summary>
/// P0：<see cref="EntityWorld"/> 之 Spawn/掛件/銷毀語意，以及 <see cref="ComponentRegistry"/> 靜態工廠路徑。
/// </summary>
/// <remarks>
/// 建構與 <see cref="IDisposable.Dispose"/> 皆呼叫 <see cref="ComponentRegistry.ClearForTesting"/>，避免靜態表汙染其他測試類別。
/// </remarks>
public sealed class EntityWorldTests : IDisposable
{
	/// <summary>每個測試方法執行前清空全域工廠表。</summary>
	public EntityWorldTests()
	{
		ComponentRegistry.ClearForTesting();
	}

	public void Dispose()
	{
		// 與建構子對稱，確保 Xunit 平行執行時不殘留工廠委派。
		ComponentRegistry.ClearForTesting();
	}

	[Fact]
	public void Spawn_AddComponents_Destroy_InvalidatesHandle()
	{
		// 編排：Spawn → 掛 Transform/Name → 讀回欄位 → Destroy → 驗證 IsAlive 與 TryGet 皆失敗（世代防護）。
		var world = new EntityWorld();
		var e = world.Spawn();
		Assert.True(e.IsValid);
		Assert.True(world.IsAlive(e));

		// 元件建構子須傳入當前 Handle，與世界內字典鍵一致。
		world.AddComponent(e, new TransformComponent(e) { X = 1, Y = 2, Z = 3 });
		world.AddComponent(e, new NameComponent(e) { DisplayName = "Unit" });

		Assert.True(world.TryGetComponent(e, out TransformComponent? t));
		Assert.Equal(1f, t!.X);
		Assert.True(world.TryGetComponent(e, out NameComponent? n));
		Assert.Equal("Unit", n!.DisplayName);

		world.Destroy(e);
		Assert.False(world.IsAlive(e));
		Assert.False(world.TryGetComponent(e, out TransformComponent? _));
	}

	[Fact]
	public void ComponentRegistry_TryCreate_UsesFactory()
	{
		// 靜態工廠表：註冊 NameComponent 後，TryCreate 應產生實例；再掛入世界應與 TryGet 為同一引用。
		ComponentRegistry.Register(h => new NameComponent(h));
		var world = new EntityWorld();
		var e = world.Spawn();
		var created = ComponentRegistry.TryCreate(e, typeof(NameComponent));
		Assert.NotNull(created);
		Assert.IsType<NameComponent>(created);
		// AddComponent 需要具體衍生型別實例；TryCreate 回傳基底型別，故向下轉型。
		world.AddComponent(e, (NameComponent)created);
		Assert.True(world.TryGetComponent(e, out NameComponent? nc));
		Assert.Same(created, nc);
	}

	[Fact]
	public void HasComponent_reflects_mount_and_destroy()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		Assert.False(world.HasComponent<TransformComponent>(e));

		world.AddComponent(e, new TransformComponent(e));
		Assert.True(world.HasComponent<TransformComponent>(e));
		Assert.False(world.HasComponent<NameComponent>(e));

		world.Destroy(e);
		Assert.False(world.HasComponent<TransformComponent>(e));
	}

	[Fact]
	public void GetComponent_returns_same_instance_as_TryGet_when_present()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		var tc = new TransformComponent(e) { X = 5 };
		world.AddComponent(e, tc);
		Assert.True(world.TryGetComponent(e, out TransformComponent? viaTry));
		Assert.Same(tc, world.GetComponent<TransformComponent>(e));
		Assert.Same(tc, viaTry);
	}

	[Fact]
	public void GetComponent_throws_when_entity_dead_or_missing_component()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		var exMissing = Assert.Throws<InvalidOperationException>(() => world.GetComponent<TransformComponent>(e));
		Assert.Contains("未掛載", exMissing.Message);

		world.AddComponent(e, new TransformComponent(e));
		world.Destroy(e);
		var exDead = Assert.Throws<InvalidOperationException>(() => world.GetComponent<TransformComponent>(e));
		Assert.Contains("銷毀", exDead.Message);
	}

	[Fact]
	public void Has_Get_and_TryGet_respect_generation_mismatch()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		world.AddComponent(e, new NameComponent(e) { DisplayName = "ok" });
		var forged = new EntityHandle { Index = e.Index, Generation = e.Generation + 1 };

		Assert.False(world.HasComponent<NameComponent>(forged));
		Assert.Throws<InvalidOperationException>(() => world.GetComponent<NameComponent>(forged));
		Assert.False(world.TryGetComponent(forged, out NameComponent? _));
	}

	[Fact]
	public void GetComponentCount_is_zero_for_dead_or_empty_and_matches_mounts()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		Assert.Equal(0, world.GetComponentCount(e));

		world.AddComponent(e, new TransformComponent(e));
		Assert.Equal(1, world.GetComponentCount(e));
		world.AddComponent(e, new NameComponent(e));
		Assert.Equal(2, world.GetComponentCount(e));

		world.RemoveComponent<TransformComponent>(e);
		Assert.Equal(1, world.GetComponentCount(e));

		world.Destroy(e);
		Assert.Equal(0, world.GetComponentCount(e));
	}

	[Fact]
	public void GetComponentCount_returns_zero_for_forged_generation()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		world.AddComponent(e, new NameComponent(e));
		var forged = new EntityHandle { Index = e.Index, Generation = e.Generation + 1 };
		Assert.Equal(0, world.GetComponentCount(forged));
	}

	[Fact]
	public void NonGeneric_Has_and_TryGet_match_generic_reference()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		var tc = new TransformComponent(e) { X = 7 };
		world.AddComponent(e, tc);

		Assert.True(world.HasComponent(e, typeof(TransformComponent)));
		Assert.True(world.TryGetComponent(e, typeof(TransformComponent), out var viaType));
		Assert.Same(tc, viaType);
		Assert.True(world.TryGetComponent(e, out TransformComponent? viaGeneric));
		Assert.Same(tc, viaGeneric);
	}

	[Fact]
	public void NonGeneric_Has_TryGet_false_when_missing_dead_or_forged_generation()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		Assert.False(world.HasComponent(e, typeof(NameComponent)));
		Assert.False(world.TryGetComponent(e, typeof(NameComponent), out var _));

		world.AddComponent(e, new NameComponent(e));
		world.Destroy(e);
		Assert.False(world.HasComponent(e, typeof(NameComponent)));
		Assert.False(world.TryGetComponent(e, typeof(NameComponent), out var _2));

		var forged = new EntityHandle { Index = e.Index, Generation = e.Generation + 1 };
		Assert.False(world.HasComponent(forged, typeof(NameComponent)));
		Assert.False(world.TryGetComponent(forged, typeof(NameComponent), out var _3));
	}

	[Fact]
	public void NonGeneric_Throws_ArgumentNull_for_null_type()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		Assert.Throws<ArgumentNullException>(() => world.HasComponent(e, null!));
		Assert.Throws<ArgumentNullException>(() => world.TryGetComponent(e, null!, out var _));
		Assert.Throws<ArgumentNullException>(() => world.GetComponent(e, null!));
		Assert.Throws<ArgumentNullException>(() => world.RemoveComponent(e, null!));
	}

	[Fact]
	public void NonGeneric_Throws_ArgumentException_for_non_VGComponent_type()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		var exH = Assert.Throws<ArgumentException>(() => world.HasComponent(e, typeof(string)));
		Assert.Contains("VGComponent", exH.Message);
		var exT = Assert.Throws<ArgumentException>(() => world.TryGetComponent(e, typeof(string), out var _));
		Assert.Contains("VGComponent", exT.Message);
		var exG = Assert.Throws<ArgumentException>(() => world.GetComponent(e, typeof(string)));
		Assert.Contains("VGComponent", exG.Message);
		var exR = Assert.Throws<ArgumentException>(() => world.RemoveComponent(e, typeof(string)));
		Assert.Contains("VGComponent", exR.Message);
	}

	[Fact]
	public void NonGeneric_GetComponent_returns_same_instance_as_generic_when_present()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		var tc = new TransformComponent(e) { X = 11 };
		world.AddComponent(e, tc);
		Assert.Same(tc, world.GetComponent<TransformComponent>(e));
		Assert.Same(tc, world.GetComponent(e, typeof(TransformComponent)));
	}

	[Fact]
	public void NonGeneric_GetComponent_throws_same_messages_as_generic_when_dead_or_missing()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		var exMissingGeneric = Assert.Throws<InvalidOperationException>(() => world.GetComponent<TransformComponent>(e));
		var exMissingType = Assert.Throws<InvalidOperationException>(() => world.GetComponent(e, typeof(TransformComponent)));
		Assert.Equal(exMissingGeneric.Message, exMissingType.Message);

		world.AddComponent(e, new TransformComponent(e));
		world.Destroy(e);
		var exDeadGeneric = Assert.Throws<InvalidOperationException>(() => world.GetComponent<TransformComponent>(e));
		var exDeadType = Assert.Throws<InvalidOperationException>(() => world.GetComponent(e, typeof(TransformComponent)));
		Assert.Equal(exDeadGeneric.Message, exDeadType.Message);
	}

	[Fact]
	public void NonGeneric_RemoveComponent_clears_has_and_decrements_count_like_generic()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		world.AddComponent(e, new TransformComponent(e));
		world.AddComponent(e, new NameComponent(e));
		Assert.Equal(2, world.GetComponentCount(e));

		world.RemoveComponent(e, typeof(TransformComponent));
		Assert.False(world.HasComponent<TransformComponent>(e));
		Assert.False(world.HasComponent(e, typeof(TransformComponent)));
		Assert.Equal(1, world.GetComponentCount(e));
		Assert.Throws<InvalidOperationException>(() => world.GetComponent<TransformComponent>(e));
		Assert.Throws<InvalidOperationException>(() => world.GetComponent(e, typeof(TransformComponent)));

		world.RemoveComponent<NameComponent>(e);
		Assert.Equal(0, world.GetComponentCount(e));
	}

	[Fact]
	public void NonGeneric_RemoveComponent_is_silent_when_entity_dead()
	{
		var world = new EntityWorld();
		var e = world.Spawn();
		world.AddComponent(e, new TransformComponent(e));
		world.Destroy(e);
		world.RemoveComponent(e, typeof(TransformComponent));
	}
}
