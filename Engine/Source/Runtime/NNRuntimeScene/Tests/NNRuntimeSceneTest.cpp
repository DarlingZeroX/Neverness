/**
 * @file NNRuntimeSceneTest.cpp
 * @brief NNRuntimeScene Phase 1–3 单元测试。
 */

#include <gtest/gtest.h>

#include <typeindex>

#include "NNRuntimeScene/Include/Components/NNRelationshipComponent.h"
#include "NNRuntimeScene/Include/Components/NNTagComponent.h"
#include "NNRuntimeScene/Include/Components/NNTransformComponent.h"
#include "NNRuntimeScene/Include/Reflection/NNComponentRegistry.h"
#include "NNRuntimeScene/Include/Runtime/NNSceneEventBus.h"
#include "NNRuntimeScene/Include/Serialization/NNSceneSerializer.h"
#include "NNRuntimeScene/Include/Scene/NNEntity.h"
#include "NNRuntimeScene/Include/Scene/NNRuntimeScene.h"
#include "NNRuntimeScene/Include/Systems/NNTransformSystem.h"

using namespace NN::Runtime::Scene;

TEST(NNRuntimeSceneTest, CreateDestroyAndGeneration)
{
	NNRuntimeScene scene;
	const NNEntity a = scene.CreateEntity();
	ASSERT_NE(a, NNEntityInvalid);
	ASSERT_TRUE(scene.IsAlive(a));

	const auto parts = UnpackEntityHandle(a);
	EXPECT_GE(parts.Index, 1u);
	EXPECT_GE(parts.Generation, 1u);

	EXPECT_TRUE(scene.DestroyEntity(a));
	EXPECT_FALSE(scene.IsAlive(a));
	EXPECT_FALSE(scene.DestroyEntity(a));

	const NNEntity stale = a;
	EXPECT_FALSE(scene.IsAlive(stale));
}

TEST(NNRuntimeSceneTest, EmplaceTryGetRemove)
{
	NNRuntimeScene scene;
	const NNEntity e = scene.CreateEntity();
	ASSERT_TRUE(scene.IsAlive(e));

	auto& transform = scene.Emplace<NNTransformComponent>(e);
	transform.Position[0] = 3.f;
	ASSERT_NE(scene.TryGet<NNTransformComponent>(e), nullptr);
	EXPECT_FLOAT_EQ(scene.TryGet<NNTransformComponent>(e)->Position[0], 3.f);

	EXPECT_TRUE(scene.Has<NNTransformComponent>(e));
	EXPECT_TRUE(scene.Remove<NNTransformComponent>(e));
	EXPECT_FALSE(scene.Has<NNTransformComponent>(e));
}

TEST(NNRuntimeSceneTest, QueryEach)
{
	NNRuntimeScene scene;
	const NNEntity e = scene.CreateEntity();
	scene.Emplace<NNTransformComponent>(e);
	scene.Emplace<NNTagComponent>(e).Flags = 7u;

	std::size_t count = 0;
	scene.Query<NNTransformComponent, NNTagComponent>().Each(
		[&](NNEntity handle, NNTransformComponent&, NNTagComponent& tag)
		{
			EXPECT_EQ(handle, e);
			EXPECT_EQ(tag.Flags, 7u);
			++count;
		});
	EXPECT_EQ(count, 1u);
}

TEST(NNRuntimeSceneTest, ComponentRegistryBuiltin)
{
	NNRuntimeScene scene;
	auto& reg = scene.GetComponentRegistry();
	EXPECT_GE(reg.GetRegisteredCount(), 3u);
	EXPECT_NE(reg.FindTypeId(std::type_index(typeid(NNTransformComponent))), NNComponentTypeIdInvalid);
	const NNComponentFieldDesc* positionField =
		reg.GetFieldByName(reg.FindTypeId(std::type_index(typeid(NNTransformComponent))), "Position");
	ASSERT_NE(positionField, nullptr);
	EXPECT_EQ(positionField->FieldType, NNComponentFieldType::Float3);
}

TEST(NNRuntimeSceneTest, HierarchySetParent)
{
	NNRuntimeScene scene;
	const NNEntity parent = scene.CreateEntityWithDefaults();
	const NNEntity child = scene.CreateEntityWithDefaults();

	ASSERT_TRUE(scene.SetParent(child, parent));
	EXPECT_EQ(scene.GetParent(child), parent);
	EXPECT_EQ(scene.GetChildren(parent).size(), 1u);

	const NNRelationshipComponent* rel = scene.TryGet<NNRelationshipComponent>(child);
	ASSERT_NE(rel, nullptr);
	EXPECT_EQ(rel->Depth, 1u);
}

TEST(NNRuntimeSceneTest, HierarchyRejectsCycle)
{
	NNRuntimeScene scene;
	const NNEntity a = scene.CreateEntityWithDefaults();
	const NNEntity b = scene.CreateEntityWithDefaults();
	ASSERT_TRUE(scene.SetParent(b, a));
	EXPECT_FALSE(scene.SetParent(a, b));
}

TEST(NNRuntimeSceneTest, SystemSchedulerTicksTransform)
{
	NNRuntimeScene scene;
	const NNEntity e = scene.CreateEntityWithDefaults();
	(void)e;
	EXPECT_GE(scene.GetSystemScheduler().GetRegisteredCount(), 3u);
	scene.TickSystems(0.016f);
	EXPECT_GE(scene.GetTransformSystem().GetLastTickedTransformCount(), 1u);
}

TEST(NNRuntimeSceneTest, EventBusEntityCreated)
{
	NNRuntimeScene scene;
	int createdCount = 0;
	scene.GetEventBus().Subscribe(
		[&](const NNSceneEntityEvent& evt)
		{
			if (evt.Type == NNSceneEventType::EntityCreated)
			{
				++createdCount;
			}
		});
	(const void)scene.CreateEntity();
	EXPECT_EQ(createdCount, 1);
}

TEST(NNRuntimeSceneTest, DirtyTrackerConsume)
{
	NNRuntimeScene scene;
	const NNEntity e = scene.CreateEntity();
	scene.GetDirtyTracker().MarkEntityDirty(e);
	auto dirty = scene.GetDirtyTracker().ConsumeDirtyEntities();
	ASSERT_EQ(dirty.size(), 1u);
	EXPECT_EQ(dirty[0], e);
	EXPECT_FALSE(scene.GetDirtyTracker().IsEntityDirty(e));
}

TEST(NNRuntimeSceneTest, SerializeRoundTrip)
{
	NNRuntimeScene scene;
	const NNEntity e = scene.CreateEntityWithDefaults();
	if (NNTransformComponent* t = scene.TryGet<NNTransformComponent>(e))
	{
		t->Position[0] = 42.f;
		t->Scale[0] = 2.f;
	}

	const std::vector<std::uint8_t> blob = NNSceneSerializer::Serialize(scene);
	ASSERT_FALSE(blob.empty());

	NNRuntimeScene loaded;
	ASSERT_TRUE(NNSceneSerializer::Deserialize(loaded, blob));
	ASSERT_EQ(loaded.GetRegistry().view<NNTransformComponent>().size(), 1u);

	loaded.Query<NNTransformComponent>().Each(
		[](NNEntity /*handle*/, const NNTransformComponent& transform)
		{
			EXPECT_FLOAT_EQ(transform.Position[0], 42.f);
			EXPECT_FLOAT_EQ(transform.Scale[0], 2.f);
		});
}
