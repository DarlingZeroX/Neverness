/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#include "Scene/SceneSystem.h"
#include "Scene/SceneFactory.h"
#include "Scene/ComponentSerializer.h"
#include "Scene/GameActorFactory.h"
#include "NNRuntimeAsset/Interface/SceneSerializerFactory.h"

namespace VisionGal
{
	void SceneSystem::Initialize()
	{		
		// 注册默认的场景工厂
		SceneFactoryRegistry::Register(new SceneFactory());

		// 注册默认的游戏对象工厂
		GameActorFactoryRegistry::Register(GetGameActorFactory());

		// 注册默认的场景序列化器
		SceneSerializerRegistry::RegisterSegmentSerializer(
			TransformComponentSerializer{}.GetSegmentType() , 
			MakeRef<TransformComponentSerializer>()
		);
		SceneSerializerRegistry::RegisterSegmentSerializer(
			CameraComponentSerializer{}.GetSegmentType() , 
			MakeRef<CameraComponentSerializer>()
		);
		SceneSerializerRegistry::RegisterSegmentSerializer(
			"SpriteRenderer" , 
			MakeRef<SpriteRendererComponentSerializer>()
		);
		SceneSerializerRegistry::RegisterSegmentSerializer(
			ScriptComponentSerializer{}.GetSegmentType() , 
			MakeRef<ScriptComponentSerializer>()
		);
		SceneSerializerRegistry::RegisterSegmentSerializer(
			AudioSourceComponentSerializer{}.GetSegmentType() , 
			MakeRef<AudioSourceComponentSerializer>()
		);
		SceneSerializerRegistry::RegisterSegmentSerializer(
			VideoPlayerComponentSerializer{}.GetSegmentType() , 
			MakeRef<VideoPlayerComponentSerializer>() 
		);
		SceneSerializerRegistry::RegisterSegmentSerializer(
			RmlUIDocumentComponentSerializer{}.GetSegmentType() , 
			MakeRef<RmlUIDocumentComponentSerializer>()
		);
		
		//SceneSerializerRegistry::RegisterSegmentSerializer(
		//	GalGame::GalGameEngineComponentSerializer{}.GetSegmentType() , 
		//	MakeRef<GalGame::GalGameEngineComponentSerializer>()
		//);
	}
}
