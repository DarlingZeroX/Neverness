/**
 * @file BuiltinComponentRegistration.cpp
 * @brief 内置组件静态注册（含 Phase 4-B 字段反射元数据）。
 */

#include "Components/NNAudioSourceComponent.h"
#include "Components/NNCameraComponent.h"
#include "Components/NNRelationshipComponent.h"
#include "Components/NNSpriteRendererComponent.h"
#include "Components/NNTagComponent.h"
#include "Components/NNTransformComponent.h"
#include "Components/NNVideoPlayerComponent.h"
#include "Components/NNRmlUIDocumentComponent.h"
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

NN_REGISTER_COMPONENT(
		NN::Runtime::Scene::NNCameraComponent,
		"Camera",
		NN_FIELD(NN::Runtime::Scene::NNCameraComponent, Projection, UInt32),
		NN_FIELD(NN::Runtime::Scene::NNCameraComponent, NearPlane, Float),
		NN_FIELD(NN::Runtime::Scene::NNCameraComponent, FarPlane, Float),
		NN_FIELD(NN::Runtime::Scene::NNCameraComponent, _padding0, UInt32),
		NN_FIELD(NN::Runtime::Scene::NNCameraComponent, FovY, Float),
		NN_FIELD(NN::Runtime::Scene::NNCameraComponent, AspectRatio, Float),
		NN_FIELD(NN::Runtime::Scene::NNCameraComponent, OrthoWidth, Float),
		NN_FIELD(NN::Runtime::Scene::NNCameraComponent, OrthoHeight, Float),
		NN_FIELD(NN::Runtime::Scene::NNCameraComponent, ProjectionMatrix, Float4x4));

NN_REGISTER_COMPONENT(
	NN::Runtime::Scene::NNSpriteRendererComponent,
	"SpriteRenderer",
	NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, TextureAsset, Guid),
	NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, MaterialAsset, Guid),
	NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, Color, Float4),
	NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, UvRect, Float4),
	NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, Layer, UInt32),
	NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, SortOrder, UInt32),
	NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, Blend, UInt32),
	NN_FIELD(NN::Runtime::Scene::NNSpriteRendererComponent, Flags, UInt32));
NN_REGISTER_COMPONENT(
		NN::Runtime::Scene::NNAudioSourceComponent,
		"AudioSource",
		NN_FIELD(NN::Runtime::Scene::NNAudioSourceComponent, AudioClipAsset, Guid),
		NN_FIELD(NN::Runtime::Scene::NNAudioSourceComponent, RuntimePlayerId, UInt32),
		NN_FIELD(NN::Runtime::Scene::NNAudioSourceComponent, Volume, Float),
		NN_FIELD(NN::Runtime::Scene::NNAudioSourceComponent, Pitch, Float),
		NN_FIELD(NN::Runtime::Scene::NNAudioSourceComponent, MinDistance, Float),
		NN_FIELD(NN::Runtime::Scene::NNAudioSourceComponent, MaxDistance, Float),
		NN_FIELD(NN::Runtime::Scene::NNAudioSourceComponent, Flags, UInt32));

NN_REGISTER_COMPONENT(
		NN::Runtime::Scene::NNVideoPlayerComponent,
		"VideoPlayer",
		NN_FIELD(NN::Runtime::Scene::NNVideoPlayerComponent, VideoClipAsset, Guid),
		NN_FIELD(NN::Runtime::Scene::NNVideoPlayerComponent, RuntimePlayerId, UInt32),
		NN_FIELD(NN::Runtime::Scene::NNVideoPlayerComponent, VideoTextureId, UInt32),
		NN_FIELD(NN::Runtime::Scene::NNVideoPlayerComponent, Volume, Float),
		NN_FIELD(NN::Runtime::Scene::NNVideoPlayerComponent, Flags, UInt32),
		NN_FIELD(NN::Runtime::Scene::NNVideoPlayerComponent, TargetSprite, Guid));

NN_REGISTER_COMPONENT(
		NN::Runtime::Scene::NNRmlUIDocumentComponent,
		"RmlUIDocument",
		NN_FIELD(NN::Runtime::Scene::NNRmlUIDocumentComponent, DocumentAsset, Guid),
		NN_FIELD(NN::Runtime::Scene::NNRmlUIDocumentComponent, Flags, UInt32),
		NN_FIELD(NN::Runtime::Scene::NNRmlUIDocumentComponent, SortOrder, UInt32));
} // namespace
