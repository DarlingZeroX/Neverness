/**
 * @file NNRuntimeSceneTest.cpp
 * @brief NNRuntimeScene Phase 1–4-B 单元测试。
 */

#include <gtest/gtest.h>

#include <cmath>
#include <typeindex>

#include <NNCore/Interface/HCoreTypes.h>
#include <NNCore/Include/Math/GLM/ext/matrix_transform.hpp>
#include <NNCore/Include/Math/GLM/gtc/quaternion.hpp>
#include <NNCore/Include/Math/GLM/gtc/type_ptr.hpp>

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
using namespace NN::Core;

namespace
{
/// 从 Position/Rotation/Scale 构建局部矩阵（与 NNTransformSystem::BuildLocalMatrix 一致）
glm::mat4 BuildTestLocalMatrix(const float3& pos, const quaternion& rot, const float3& scl)
{
	const glm::mat4 T = glm::translate(glm::identity<glm::mat4>(), pos);
	const glm::mat4 R = glm::mat4_cast(rot);
	const glm::mat4 S = glm::scale(glm::identity<glm::mat4>(), scl);
	return T * R * S;
}

/// 矩阵逐元素近似比较
void ExpectMatrixNear(const matrix& actual, const glm::mat4& expected, float tolerance = 1e-4f)
{
	const float* a = glm::value_ptr(actual);
	const float* e = glm::value_ptr(expected);
	for (int i = 0; i < 16; ++i)
	{
		EXPECT_NEAR(a[i], e[i], tolerance)
			<< "Matrix element [" << (i / 4) << "][" << (i % 4) << "] mismatch";
	}
}
} // namespace

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
	transform.Position.x = 3.f;
	ASSERT_NE(scene.TryGet<NNTransformComponent>(e), nullptr);
	EXPECT_FLOAT_EQ(scene.TryGet<NNTransformComponent>(e)->Position.x, 3.f);

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

	// 验证 Position 字段注册为 Float3
	const NNComponentFieldDesc* positionField =
		reg.GetFieldByName(reg.FindTypeId(std::type_index(typeid(NNTransformComponent))), "Position");
	ASSERT_NE(positionField, nullptr);
	EXPECT_EQ(positionField->FieldType, NNComponentFieldType::Float3);

	// 验证 Rotation 字段注册为 Quaternion
	const NNComponentFieldDesc* rotationField =
		reg.GetFieldByName(reg.FindTypeId(std::type_index(typeid(NNTransformComponent))), "Rotation");
	ASSERT_NE(rotationField, nullptr);
	EXPECT_EQ(rotationField->FieldType, NNComponentFieldType::Quaternion);

	// 验证 WorldMatrix 字段注册为 Float4x4
	const NNComponentFieldDesc* worldMatrixField =
		reg.GetFieldByName(reg.FindTypeId(std::type_index(typeid(NNTransformComponent))), "WorldMatrix");
	ASSERT_NE(worldMatrixField, nullptr);
	EXPECT_EQ(worldMatrixField->FieldType, NNComponentFieldType::Float4x4);
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
		t->Position.x = 42.f;
		t->Scale.x = 2.f;
	}

	const std::vector<std::uint8_t> blob = NNSceneSerializer::Serialize(scene);
	ASSERT_FALSE(blob.empty());

	NNRuntimeScene loaded;
	ASSERT_TRUE(NNSceneSerializer::Deserialize(loaded, blob));
	ASSERT_EQ(loaded.GetRegistry().view<NNTransformComponent>().size(), 1u);

	loaded.Query<NNTransformComponent>().Each(
		[](NNEntity /*handle*/, const NNTransformComponent& transform)
		{
			EXPECT_FLOAT_EQ(transform.Position.x, 42.f);
			EXPECT_FLOAT_EQ(transform.Scale.x, 2.f);
		});
}

/**
 * @brief 诊断测试：验证 TransformComponent 的 WorldMatrix 字段可正确读写。
 */
TEST(NNRuntimeSceneTest, TransformWorldMatrixFieldAccess)
{
	NNRuntimeScene scene;
	const NNEntity e = scene.CreateEntityWithDefaults();

	NNTransformComponent* t = scene.TryGet<NNTransformComponent>(e);
	ASSERT_NE(t, nullptr);

	// 验证 WorldMatrix 初始值为单位阵
	const glm::mat4 identity = glm::identity<glm::mat4>();
	ExpectMatrixNear(t->WorldMatrix, identity);

	// Tick 后验证根节点世界矩阵被正确计算（纯平移）
	t->Position = NN::Core::float3(5.f, 0.f, 0.f);
	scene.TickSystems(0.016f);

	EXPECT_FLOAT_EQ(t->WorldMatrix[3].x, 5.f); // 平移.x（GLM 列主序，第4列）
	EXPECT_FLOAT_EQ(t->WorldMatrix[3].y, 0.f);
	EXPECT_FLOAT_EQ(t->WorldMatrix[3].z, 0.f);
	EXPECT_FLOAT_EQ(t->WorldMatrix[3].w, 1.f);
}

/**
 * @brief 验证单亲层级世界矩阵：Child.WorldMatrix = Parent.WorldMatrix * Child.LocalMatrix。
 *
 * 场景：Parent 平移 (1,2,3)，子节点缩放 (2,2,2)。
 * 预期：子节点世界矩阵 = Parent 的局部矩阵 * Child 的局部矩阵。
 */
TEST(NNRuntimeSceneTest, TransformWorldMatrixSingleParent)
{
	NNRuntimeScene scene;
	const NNEntity parent = scene.CreateEntityWithDefaults();
	const NNEntity child = scene.CreateEntityWithDefaults();
	ASSERT_TRUE(scene.SetParent(child, parent));

	// 诊断：验证层级关系正确
	EXPECT_EQ(scene.GetParent(child), parent);
	const auto children = scene.GetChildren(parent);
	ASSERT_EQ(children.size(), 1u);
	EXPECT_EQ(children[0], child);

	// 设置父节点变换（纯平移）
	NNTransformComponent* parentTransform = scene.TryGet<NNTransformComponent>(parent);
	ASSERT_NE(parentTransform, nullptr);
	parentTransform->Position = NN::Core::float3(1.f, 2.f, 3.f);

	// 设置子节点变换（纯缩放）
	NNTransformComponent* childTransform = scene.TryGet<NNTransformComponent>(child);
	ASSERT_NE(childTransform, nullptr);
	childTransform->Scale = NN::Core::float3(2.f, 2.f, 2.f);

	// Tick 系统计算世界矩阵
	scene.TickSystems(0.016f);

	// 诊断：验证 TransformSystem 处理了实体
	EXPECT_GE(scene.GetTransformSystem().GetLastTickedTransformCount(), 1u);

	// 计算预期矩阵
	const glm::mat4 expectedParentLocal =
		BuildTestLocalMatrix(parentTransform->Position, parentTransform->Rotation, parentTransform->Scale);
	const glm::mat4 expectedChildLocal =
		BuildTestLocalMatrix(childTransform->Position, childTransform->Rotation, childTransform->Scale);
	const glm::mat4 expectedChildWorld = expectedParentLocal * expectedChildLocal;

	// 验证父节点世界矩阵（根节点，世界=局部）
	ExpectMatrixNear(parentTransform->WorldMatrix, expectedParentLocal);

	// 验证子节点世界矩阵
	ExpectMatrixNear(childTransform->WorldMatrix, expectedChildWorld);
}

/**
 * @brief 验证多层级世界矩阵：三级父子链（Root → Mid → Leaf）。
 *
 * 场景：
 *   Root: 平移 (10, 0, 0)，绕 Y 轴旋转 90 度
 *   Mid:  平移 (0, 5, 0)，缩放 (2, 2, 2)
 *   Leaf: 平移 (0, 0, 3)
 *
 * 预期：Leaf.WorldMatrix = Root.Local * Mid.Local * Leaf.Local
 */
TEST(NNRuntimeSceneTest, TransformWorldMatrixMultiLevel)
{
	NNRuntimeScene scene;
	const NNEntity root = scene.CreateEntityWithDefaults();
	const NNEntity mid = scene.CreateEntityWithDefaults();
	const NNEntity leaf = scene.CreateEntityWithDefaults();

	ASSERT_TRUE(scene.SetParent(mid, root));
	ASSERT_TRUE(scene.SetParent(leaf, mid));

	// Root：平移 + 绕 Y 轴旋转 90 度
	NNTransformComponent* rootT = scene.TryGet<NNTransformComponent>(root);
	rootT->Position = NN::Core::float3(10.f, 0.f, 0.f);
	rootT->Rotation = glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f));

	// Mid：平移 + 缩放
	NNTransformComponent* midT = scene.TryGet<NNTransformComponent>(mid);
	midT->Position = NN::Core::float3(0.f, 5.f, 0.f);
	midT->Scale = NN::Core::float3(2.f, 2.f, 2.f);

	// Leaf：仅平移
	NNTransformComponent* leafT = scene.TryGet<NNTransformComponent>(leaf);
	leafT->Position = NN::Core::float3(0.f, 0.f, 3.f);

	// Tick 系统
	scene.TickSystems(0.016f);

	// 计算预期矩阵链
	const glm::mat4 expectedRootLocal =
		BuildTestLocalMatrix(rootT->Position, rootT->Rotation, rootT->Scale);
	const glm::mat4 expectedMidLocal =
		BuildTestLocalMatrix(midT->Position, midT->Rotation, midT->Scale);
	const glm::mat4 expectedLeafLocal =
		BuildTestLocalMatrix(leafT->Position, leafT->Rotation, leafT->Scale);

	const glm::mat4 expectedMidWorld = expectedRootLocal * expectedMidLocal;
	const glm::mat4 expectedLeafWorld = expectedMidWorld * expectedLeafLocal;

	// 逐级验证
	ExpectMatrixNear(rootT->WorldMatrix, expectedRootLocal);
	ExpectMatrixNear(midT->WorldMatrix, expectedMidWorld);
	ExpectMatrixNear(leafT->WorldMatrix, expectedLeafWorld);
}

/**
 * @brief 验证根节点（无父子关系）的世界矩阵等于局部矩阵。
 */
TEST(NNRuntimeSceneTest, TransformWorldMatrixRootOnly)
{
	NNRuntimeScene scene;
	const NNEntity e = scene.CreateEntityWithDefaults();

	NNTransformComponent* t = scene.TryGet<NNTransformComponent>(e);
	t->Position = NN::Core::float3(5.f, 10.f, 0.f);
	t->Rotation = glm::angleAxis(glm::radians(45.f), glm::vec3(0.f, 0.f, 1.f));
	t->Scale = NN::Core::float3(3.f, 1.f, 1.f);

	scene.TickSystems(0.016f);

	const glm::mat4 expected = BuildTestLocalMatrix(t->Position, t->Rotation, t->Scale);
	ExpectMatrixNear(t->WorldMatrix, expected);
}

/**
 * @brief 验证 FNV-1a name hash 跨注册顺序稳定性与 round-trip。
 *
 * 编译期验证：name hash 是 constexpr，可用 static_assert 确认非零且唯一。
 * 运行时验证：序列化/反序列化仍能正确恢复组件数据。
 */
TEST(NNRuntimeSceneTest, SerializeRoundTripStableTypeId)
{
	// 编译期验证：FNV-1a name hash 为常量表达式
	constexpr auto hashTransform = fnv1a_64("Transform");
	constexpr auto hashRelationship = fnv1a_64("Relationship");
	constexpr auto hashTag = fnv1a_64("Tag");

	static_assert(hashTransform != 0, "Transform hash must be nonzero");
	static_assert(hashRelationship != 0, "Relationship hash must be nonzero");
	static_assert(hashTag != 0, "Tag hash must be nonzero");
	static_assert(hashTransform != hashRelationship, "Transform and Relationship hashes must differ");
	static_assert(hashTransform != hashTag, "Transform and Tag hashes must differ");
	static_assert(hashRelationship != hashTag, "Relationship and Tag hashes must differ");

	// 运行时验证：哈希在多次调用间一致（确定性）
	EXPECT_EQ(fnv1a_64("Transform"), hashTransform);
	EXPECT_EQ(fnv1a_64("Relationship"), hashRelationship);
	EXPECT_EQ(fnv1a_64("Tag"), hashTag);

	// 验证注册表中的 TypeId 等于 name hash
	NNRuntimeScene scene;
	const NNComponentRegistry& registry = scene.GetComponentRegistry();

	const NNComponentTypeId transformId = registry.FindTypeId(std::type_index(typeid(NNTransformComponent)));
	const NNComponentTypeId relationshipId = registry.FindTypeId(std::type_index(typeid(NNRelationshipComponent)));
	const NNComponentTypeId tagId = registry.FindTypeId(std::type_index(typeid(NNTagComponent)));

	EXPECT_EQ(transformId, hashTransform);
	EXPECT_EQ(relationshipId, hashRelationship);
	EXPECT_EQ(tagId, hashTag);

	// 验证 FindDescByNameHash 能正确查找
	const NNComponentTypeDesc* transformDesc = registry.FindDescByNameHash(hashTransform);
	ASSERT_NE(transformDesc, nullptr);
	EXPECT_STREQ(transformDesc->NameUtf8, "Transform");

	const NNComponentTypeDesc* relationshipDesc = registry.FindDescByNameHash(hashRelationship);
	ASSERT_NE(relationshipDesc, nullptr);
	EXPECT_STREQ(relationshipDesc->NameUtf8, "Relationship");

	// 验证 round-trip（序列化写入 nameHash，反序列化按 nameHash 查表）
	const NNEntity e = scene.CreateEntityWithDefaults();
	if (NNTransformComponent* t = scene.TryGet<NNTransformComponent>(e))
	{
		t->Position.x = 42.f;
		t->Scale.x = 2.f;
	}

	const std::vector<std::uint8_t> blob = NNSceneSerializer::Serialize(scene);
	ASSERT_FALSE(blob.empty());

	// 验证 VGSC 格式版本为 2（FNV-1a name hash 格式）
	EXPECT_EQ(NNSceneSerializer::kFormatVersion, 2u);

	NNRuntimeScene loaded;
	ASSERT_TRUE(NNSceneSerializer::Deserialize(loaded, blob));
	ASSERT_EQ(loaded.GetRegistry().view<NNTransformComponent>().size(), 1u);

	loaded.Query<NNTransformComponent>().Each(
		[](NNEntity, const NNTransformComponent& transform)
		{
			EXPECT_FLOAT_EQ(transform.Position.x, 42.f);
			EXPECT_FLOAT_EQ(transform.Scale.x, 2.f);
		});
}
