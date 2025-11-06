#pragma once

#include "OpenGL.h"
#include<vector>
#include<iostream>

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	class Bindable
	{
	public:
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual ~Bindable() = default;
	};

	class Renderable
	{
	public:
		virtual unsigned int GetRendererID() const noexcept = 0;
		virtual ~Renderable() = default;
	};
	 
VISIONGAL_OPENGL_NAMESPACE_END