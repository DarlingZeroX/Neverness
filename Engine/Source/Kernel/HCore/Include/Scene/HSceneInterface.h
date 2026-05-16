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
#include "entt.hpp"

namespace Horizon
{
	using HECS = entt::registry;

	struct HSceneInterface
	{
		virtual ~HSceneInterface() = default;

		virtual HECS* GetWorld() = 0;

		virtual void Update() {};
	};


}