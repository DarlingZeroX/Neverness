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
