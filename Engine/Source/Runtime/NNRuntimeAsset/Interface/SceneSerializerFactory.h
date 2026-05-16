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

#pragma once
#include "../VGAssetConfig.h"
#include "ISceneSerializer.h"

namespace VisionGal
{
	class VG_ASSET_API SceneSerializerRegistry
	{
	public:
		//static void Register(ISceneSerializer* factory);
		static void RegisterSegmentSerializer(const String& segmentType, const Ref<ISceneSegmentSerializer>& serializer);

		static Ref<ISceneSerializer> GetSerializer();

	private:
		static SceneSerializerRegistry& Get();

		std::unordered_map<String, Ref<ISceneSegmentSerializer>> m_SegmentSerializers;
	};
}
