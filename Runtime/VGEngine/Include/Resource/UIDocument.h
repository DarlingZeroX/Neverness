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
#include "../EngineConfig.h"
#include "../Core/Core.h"
#include <RmlUi/Core.h>
#include "../Lua/sol2/sol.hpp"

namespace VisionGal
{
	class VG_ENGINE_API RmlUIDocument : public VGEngineResource
	{
	public:
		RmlUIDocument();
		~RmlUIDocument() override;

		void Close();
		void AddUpdateCallback(const sol::function& callback);
		void Update();

		Rml::ElementDocument* document = nullptr;
		bool isClosed = false;
		std::vector<sol::function> m_LuaUpdateCallbacks;
	};
}


