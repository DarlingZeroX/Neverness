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


