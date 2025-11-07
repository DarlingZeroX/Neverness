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

#include "Graphics/OpenGL/Effect.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	void RasterStates::CullNone(bool enable)
	{
		if (enable)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
		}
	}
	void RasterStates::CullBack(bool enable)
	{
		if (enable)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
	}
	void RasterStates::CullFront(bool enable)
	{
		if (enable)
		{
			glEnable(GL_CULL_FACE); 
			glCullFace(GL_FRONT);
		}
	}
	void RasterStates::Wireframe(bool enable)
	{
		if (enable)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
	void RasterStates::DepthTest(bool enable)
	{
		if (enable)
		{
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LESS);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
	}

	void RasterStates::GammaCorrect(bool enable)
	{
		if (enable)
		{
			glEnable(GL_FRAMEBUFFER_SRGB);
		}
		else
		{
			glDisable(GL_FRAMEBUFFER_SRGB);
		}
	}

	void RasterStates::Blend(bool enable)
	{
		if (enable)
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}

	void RasterStates::StencilTest(bool enable)
	{
		if (enable)
		{
			glEnable(GL_STENCIL_TEST);
		}
		else
		{
			glDisable(GL_STENCIL_TEST);
		}
	}

VISIONGAL_OPENGL_NAMESPACE_END


