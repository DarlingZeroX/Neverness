#pragma once
#include "../../EngineConfig.h"
#include "../../Core/Core.h"

namespace VisionGal::VGFX
{
	struct IMesh
	{
		virtual ~IMesh() = default;
	};

	struct IStaticMesh: public IMesh
	{
		virtual void Render() = 0;
	};
} 