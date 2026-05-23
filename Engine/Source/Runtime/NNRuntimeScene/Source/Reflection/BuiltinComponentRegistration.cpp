/**
 * @file BuiltinComponentRegistration.cpp
 * @brief 内置组件静态注册（含 Phase 4-B 字段反射元数据）。
 */

#include "Components/NNRelationshipComponent.h"
#include "Components/NNTagComponent.h"
#include "Components/NNTransformComponent.h"
#include "Reflection/NNComponentRegistry.h"

namespace
{
NN_REGISTER_COMPONENT(
	NN::Runtime::Scene::NNTransformComponent,
	"Transform",
	NN_FIELD(NN::Runtime::Scene::NNTransformComponent, Position, Float3),
	NN_FIELD(NN::Runtime::Scene::NNTransformComponent, Rotation, Quaternion),
	NN_FIELD(NN::Runtime::Scene::NNTransformComponent, Scale, Float3),
	NN_FIELD(NN::Runtime::Scene::NNTransformComponent, WorldMatrix, Float4x4));

NN_REGISTER_COMPONENT(
	NN::Runtime::Scene::NNRelationshipComponent,
	"Relationship",
	NN_FIELD(NN::Runtime::Scene::NNRelationshipComponent, Parent, Entity),
	NN_FIELD(NN::Runtime::Scene::NNRelationshipComponent, ChildCount, UInt32),
	NN_FIELD(NN::Runtime::Scene::NNRelationshipComponent, Depth, UInt32));

NN_REGISTER_COMPONENT(
	NN::Runtime::Scene::NNTagComponent,
	"Tag",
	NN_FIELD(NN::Runtime::Scene::NNTagComponent, Flags, UInt32),
	NN_FIELD(NN::Runtime::Scene::NNTagComponent, Name, CharArray));
} // namespace
