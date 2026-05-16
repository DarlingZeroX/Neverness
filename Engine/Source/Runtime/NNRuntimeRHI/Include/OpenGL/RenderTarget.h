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

namespace NN::Runtime::OpenGL {

	class VG_RHI_API RenderTarget2D
	{
	public:
		RenderTarget2D() = default;
		RenderTarget2D(RenderTarget2D&) = delete;
		RenderTarget2D& operator=(RenderTarget2D&) = delete;
		~RenderTarget2D() = default;

		static NN::Ref<RenderTarget2D> Create(unsigned int width, unsigned int height);

		OpenGL::FrameBufferTexture* GetTexture() { return m_ColorFBT.get(); }
		OpenGL::FrameBuffer* GetFrameBuffer() { return m_FrameBuffer.get(); }

		NN::Ref<OpenGL::FrameBufferTexture> GetTextureRef() { return m_ColorFBT; }

		void CopyToTexture(VGFX::ITexture* texture);
	protected:
		void BindColorAttachments();
		bool CreateImp(unsigned int width, unsigned int height);
	private:
		NN::Ref<OpenGL::FrameBufferTexture> m_ColorFBT;
		NN::Ref<OpenGL::FrameBufferDepth> m_DepthFBT;
		NN::Ref<OpenGL::FrameBuffer> m_FrameBuffer;
	};
 
}