#include "Graphics/OpenGL/VertexArray.h"
#include "Graphics/OpenGL/ThrowMarco.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN
	class VGFXVertexElementParser
	{
	public:

#define VGFX_VERTEX_ELEMENT_PARSER_SWITCH_CASE(name) \
	case VGFX::VertexElement::TYPE::name: \
		m_Type = GetGLType(VGFX::Element<VGFX::VertexElement::name>::type);	\
		mSize = VGFX::Element<VGFX::VertexElement::name>::size;	\
		mSemantic = VGFX::Element<VGFX::VertexElement::name>::semantic; \
		break;

		VGFXVertexElementParser(VGFX::VertexElement::TYPE type)
		{
			switch (type)
			{
				VGFX_VERTEX_ELEMENT_PARSER_SWITCH_CASE(POSITION2)
				VGFX_VERTEX_ELEMENT_PARSER_SWITCH_CASE(POSITION3)
				VGFX_VERTEX_ELEMENT_PARSER_SWITCH_CASE(TEXTURE2D)
				VGFX_VERTEX_ELEMENT_PARSER_SWITCH_CASE(NORMAL)
				VGFX_VERTEX_ELEMENT_PARSER_SWITCH_CASE(TANGENT)
				VGFX_VERTEX_ELEMENT_PARSER_SWITCH_CASE(BIT_TANGENT)
				VGFX_VERTEX_ELEMENT_PARSER_SWITCH_CASE(FLOAT3_COLOR)
				VGFX_VERTEX_ELEMENT_PARSER_SWITCH_CASE(FLOAT4_COLOR)
					 
			//case VGFX::VertexElement::TYPE::POSITION2:
			//	m_Type = GetGLType(VGFX::Element<VGFX::VertexElement::POSITION2>::type);
			//	mSize = VGFX::Element<VGFX::VertexElement::POSITION2>::size;
			//	mSemantic = VGFX::Element<VGFX::VertexElement::POSITION2>::semantic;
			//	break;
			}
		}
#undef VGFXVertexElementParser_SWITCH_CASE

		static GLenum GetGLType(VGFX::ElemnetType type)
		{
			switch (type)
			{
			case VGFX::ElemnetType::FLOAT:
				return GL_FLOAT;
			}

			throw "δ???Vertex Element????";
		}

		GLenum GetElementType() { return m_Type; };
		GLint GetSize() { return mSize; };
		const char* GetSemantic() { return mSemantic; };
	private:
		GLenum m_Type;
		GLint mSize;
		const char* mSemantic;
	};

	VertexArray::~VertexArray()
	{
		Delete();
	}

	void VertexArray::Create()
	{
		m_Stride = 0;
		Gen();

		// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
		Bind();
	}

	void VertexArray::PushElement(std::vector<VertexElement::IElement*> elements)
	{
		for (auto* elem : elements)
		{
			this->ParserElement(elem->GetElementType());
		}
		AddBuffer(m_Elements, m_Stride);
	}

	void VertexArray::PushElement(std::vector<VGFX::VertexElement::IElement*> elements)
	{
		for (auto* elem : elements)
		{
			this->ParserElement(elem->GetElementType());
		}
		AddBuffer(m_Elements, m_Stride);
	}

	void VertexArray::Gen()
	{
		GL_THROW_INFO(glGenVertexArrays(1, &m_RendererID));
	}

	void VertexArray::ParserElement(VertexElement::TYPE type)
	{
		VertexElementParser parser(type);

		GLenum elem_type = parser.GetElementType();
		GLint elem_size = parser.GetSize();

		m_Elements.push_back({ elem_type,elem_size,GL_FALSE });
		m_Stride += sizeof(elem_type) * elem_size;
	}

	void VertexArray::ParserElement(VGFX::VertexElement::TYPE type)
	{
		VGFXVertexElementParser parser(type);

		GLenum elem_type = parser.GetElementType();
		GLint elem_size = parser.GetSize();

		m_Elements.push_back({ elem_type,elem_size,GL_FALSE });
		m_Stride += sizeof(elem_type) * elem_size;
	}

	//void VertexArray::PushElement()
	//{
	//	AddBuffer(m_Elements, m_Stride);
	//}

	void VertexArray::AddBuffer(const std::vector<OPENGL_INPUT_ELEMENT_DESC>& elements, unsigned int stride)
	{

		unsigned int offest = 0;

		for (unsigned int i = 0; i < elements.size(); i++)
		{
			//glVertexAttribPointer()
			const auto& element = elements[i];
			GL_THROW_INFO(glVertexAttribPointer(
				i,
				element.count,
				element.type,
				element.normlized,
				stride,
				reinterpret_cast<void*>(offest)
			));
			GL_THROW_INFO(glEnableVertexAttribArray(i));
			offest += element.count * OPENGL_INPUT_ELEMENT_DESC::GetSizeOfType(element.type);
		}

		//Unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		this->Unbind();
	}

	void VertexArray::AddBuffer(const InputLayout& layout)
	{
		const auto& elements = layout.GetElements();
		AddBuffer(elements, layout.GetStride());
	}

	VertexArray::VertexArray()
	{

	}

	void VertexArray::Bind() const
	{
		GL_THROW_INFO(glBindVertexArray(m_RendererID));
	}

	void VertexArray::Unbind() const
	{
		GL_THROW_INFO(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GL_THROW_INFO(glBindVertexArray(0));
	}

	void VertexArray::Delete()
	{
		if (m_RendererID != NULL)
		{
			GL_THROW_INFO(glDeleteVertexArrays(1, &m_RendererID););
			m_RendererID = NULL;
		}
	}

VISIONGAL_OPENGL_NAMESPACE_END

