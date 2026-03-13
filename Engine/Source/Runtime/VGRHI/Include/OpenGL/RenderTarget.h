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
#include "FrameBuffer.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	class VG_RHI_API RenderTarget2D
	{
	public:
		RenderTarget2D() = default;
		RenderTarget2D(RenderTarget2D&) = delete;
		RenderTarget2D& operator=(RenderTarget2D&) = delete;
		~RenderTarget2D() = default;

		static Ref<RenderTarget2D> Create(unsigned int width, unsigned int height);

		OpenGL::FrameBufferTexture* GetTexture() { return m_ColorFBT.get(); }
		OpenGL::FrameBuffer* GetFrameBuffer() { return m_FrameBuffer.get(); }

		Ref<OpenGL::FrameBufferTexture> GetTextureRef() { return m_ColorFBT; }

		void CopyToTexture(VGFX::ITexture* texture);
	protected:
		void BindColorAttachments();
		bool CreateImp(unsigned int width, unsigned int height);
	private:
		Ref<OpenGL::FrameBufferTexture> m_ColorFBT;
		Ref<OpenGL::FrameBufferDepth> m_DepthFBT;
		Ref<OpenGL::FrameBuffer> m_FrameBuffer;
	};
 
VISIONGAL_OPENGL_NAMESPACE_END