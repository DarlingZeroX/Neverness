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