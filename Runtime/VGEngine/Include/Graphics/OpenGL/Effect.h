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
#include "OpenGL.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	class RasterStates
	{
	public:
		//Raster State
		static void CullNone(bool enable = true);
		static void CullBack(bool enable = true);
		static void CullFront(bool enable = true);
		static void Wireframe(bool enable = true);
		static void DepthTest(bool enable = true);

		static void GammaCorrect(bool enable = true);
		static void Blend(bool enable = true);
		static void StencilTest(bool enable = true);

	};
 
VISIONGAL_OPENGL_NAMESPACE_END
