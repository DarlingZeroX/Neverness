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
//#include "../Graphics/OpenGL/ShaderProgram.h"
#include <VGRHI/Include/OpenGL/ShaderProgram.h>

namespace VisionGal {

	class Material
	{
	public:
		Material();
		~Material() = default;

		static Ref<Material> Create();

		VGFX::IShaderProgram* GetShaderProgram();
	private:

		//Ref<OpenGL::ShaderProgram> m_ShaderProgram;

		VGFX::IShaderProgram*  m_ShaderProgram;
		//Ref<>
	};

	
}
