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
#include "InputLayout.h"
#include "../../Interface/VertexElement.h"

namespace NN::Runtime::OpenGL {

	class VG_RHI_API VertexArray :public Bindable
	{
	public:
		VertexArray();
		VertexArray(VertexArray&) = delete;
		VertexArray& operator=(VertexArray&) = delete;
		~VertexArray() override;
	public:
		void Bind() const;
		void Unbind() const;
		void Delete();
		void Create();

		//Runtime Set VAO
		void PushElement(std::vector<VertexElement::IElement*> elements);
		void PushElement(std::vector<VGFX::VertexElement::IElement*> elements);
	private:
		void Gen(); 
		void ParserElement(VertexElement::TYPE type);
		void ParserElement(VGFX::VertexElement::TYPE type);
		//void PushElement();

		void AddBuffer(const InputLayout& layout);
		void AddBuffer(const std::vector<OPENGL_INPUT_ELEMENT_DESC>& elements, unsigned int stride);
	private:
		unsigned int m_Stride;
		unsigned int m_RendererID;
		std::vector<OPENGL_INPUT_ELEMENT_DESC> m_Elements;
	};

}
