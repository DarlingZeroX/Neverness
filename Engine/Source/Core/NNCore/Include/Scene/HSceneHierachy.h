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
#include "../../CoreModuleDefinitions.h"
#include "HSceneInterface.h"
#include "HBaseComponent.h"

namespace NN::Core
{
	struct CORE_MODULE_API HHierarchy
	{
		static bool DisconnectParent(HECS& registry, HEntityInterface& child, HRelationship& childInfo);

		static bool SetParent(HECS& registry, HEntityInterface& child, HEntityInterface* parent);	

		static bool SetNextSibling(HECS& registry, HEntityInterface* current, HEntityInterface* next);
	};
}