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

#include "SceneSerializerFactory.h"
#include "SceneSerializer.h"

namespace NN::Runtime
{
	void SceneSerializerRegistry::RegisterSegmentSerializer(const String& segmentType,
		const Ref<ISceneSegmentSerializer>& serializer)
	{
		Get().m_SegmentSerializers[segmentType] = serializer;
	}

	Ref<ISceneSerializer> SceneSerializerRegistry::GetSerializer()
	{
		auto serializer = MakeRef<SceneSerializer>();

		for (const auto& [segmentType, segmentSerializer] : Get().m_SegmentSerializers)
		{
			serializer->AddSegmentSerializer(segmentType, segmentSerializer);
		}

		return serializer;
	}

	SceneSerializerRegistry& SceneSerializerRegistry::Get()
	{
		static SceneSerializerRegistry instance;
		return instance;
	}
}
