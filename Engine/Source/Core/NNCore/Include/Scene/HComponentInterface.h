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
#include <string>
#include "HEntityInterface.h"

namespace NN::Core
{
	struct HComponentInterface : public HObject
	{
		HComponentInterface() = default;
		~HComponentInterface() override = default;

		virtual HEntityInterface* GetOwner() {return m_OwnerUIEntity;}
		virtual void SetOwner(HEntityInterface* entity) {m_OwnerUIEntity = entity;}
		virtual void Initialize() {}
	private:
		HEntityInterface* m_OwnerUIEntity;
	};
}