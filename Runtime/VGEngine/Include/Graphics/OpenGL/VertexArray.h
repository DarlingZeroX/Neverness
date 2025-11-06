#pragma once
#include "InputLayout.h"
#include "../Interface/VertexElement.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	class VertexArray :public Bindable
	{
	private:

	public:
		VertexArray();
		VertexArray(VertexArray&) = delete;
		VertexArray& operator=(VertexArray&) = delete;
		virtual ~VertexArray();
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

VISIONGAL_OPENGL_NAMESPACE_END
