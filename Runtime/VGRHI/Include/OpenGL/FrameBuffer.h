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
#include "Buffer.h"
#include "Texture2D.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	class VG_RHI_API FrameBuffer : public Bindable, public Renderable
	{
	public:
		FrameBuffer();
		~FrameBuffer() override;

		static Ref<FrameBuffer> Create();

		void Gen();
		void Bind() const override;
		void Unbind() const override;
		unsigned int GetRendererID() const noexcept override { return m_RendererID; }

		void Delete();
		bool CheckStatus();
		void DrawBuffers(unsigned int size);

	protected:
		void CreateImp();
	private: 
		GLuint m_RendererID;
	};

	class VG_RHI_API FrameBufferTexture : public Texture2D
	{
	public:
		enum class Type
		{
			COLOR,
			NORMAL,
			ALBEDO_SPEC,
			DEPTH24_STENCIL8,
			DEPTH_COMPONENT
		};

	public:
		FrameBufferTexture() = default;
		FrameBufferTexture(FrameBufferTexture&) = delete;
		FrameBufferTexture& operator=(FrameBufferTexture&) = delete;
		~FrameBufferTexture() override = default;

	public:
		static Ref<FrameBufferTexture> Create(unsigned int width, unsigned int height, Type type = Type::COLOR, unsigned int slot = 0);

		/// <summary>
		/// Attach Texture Attachments to currently bound framebuffer object
		/// </summary>
		virtual void AttachToFrameBuffer();

		virtual int Width();
		virtual int Height();
	protected:
		virtual void CreateFBTInline(unsigned int width, unsigned int height, Type type = Type::COLOR, unsigned int slot = 0);
	private:
		unsigned int m_FrameBufferSlot = 0;
		Type m_Type = Type::COLOR;
		unsigned int m_FrameBufferBindType = 0;
	};

	class VG_RHI_API FrameBufferDepth : public Bindable
	{
	public:
		enum class Type
		{
			DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,
			DEPTH_COMPONENT = GL_DEPTH_COMPONENT
		};
	public:
		FrameBufferDepth();
		~FrameBufferDepth() override;

		static Ref<FrameBufferDepth> Create(unsigned int width, unsigned int height, Type type = Type::DEPTH24_STENCIL8);
	public:
		/// <summary>
		/// Attach Render Buffer Object Attachment to currently bound framebuffer object
		/// </summary>
		virtual void AttachToFrameBuffer() const;
		virtual void Bind() const;
		virtual void Unbind() const;
		virtual void Delete();

		virtual void SetStorage(int width, int height);

		GLuint GetRendererID() const;
		GLenum GetAttachType() const;
		GLenum GetStorageType() const;
	protected:
		virtual bool CreateImp(
			unsigned int width = 0,
			unsigned int height = 0,
			Type type = Type::DEPTH24_STENCIL8
		);
		virtual void Gen();
	private:
		GLuint m_RenderBufferID = 0;
		Type m_Type = Type::DEPTH24_STENCIL8;
		GLenum m_AttachType = 0;
		GLenum m_StorageType = 0;
	};

VISIONGAL_OPENGL_NAMESPACE_END
