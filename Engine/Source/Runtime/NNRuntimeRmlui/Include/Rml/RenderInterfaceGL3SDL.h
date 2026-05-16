#pragma once
#include "RmlUi_Renderer_GL3.h"

namespace NN::Runtime
{
	/**
	Custom render interface example for the SDL/GL3 backend.

	Overloads the OpenGL3 render interface to load textures through SDL_image's built-in texture loading functionality.
 */
	class RenderInterface_GL3_SDL : public RenderInterface_GL3 {
	public:
		RenderInterface_GL3_SDL() {}

		Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions, const Rml::String& source) override;
	};

}
