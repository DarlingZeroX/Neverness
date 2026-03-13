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
#include "Interface.h"

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	namespace VertexElement
	{
		//Element type
		enum class TYPE
		{
			POSITION2,
			POSITION3,
			TEXTURE2D,
			NORMAL,
			TANGENT,
			BIT_TANGENT,
			FLOAT3_COLOR,
			FLOAT4_COLOR
		};

		//Interface
		class IElement
		{
		public:
			virtual TYPE GetElementType() = 0;
			virtual const std::string GetElementName() = 0;
		};
		 
		//Element class
#define ElEMENT_CLASS(class_name,entity_name)			\
		class class_name: public IElement					\
		{													\
			virtual TYPE GetElementType()					\
			{												\
				return TYPE::class_name;					\
			};												\
			virtual const std::string GetElementName()		\
			{												\
				return #class_name;							\
			};												\
		};													\
		static class_name entity_name;	//Element entity

		ElEMENT_CLASS(POSITION2, Position2)
			ElEMENT_CLASS(POSITION3, Position3)
			ElEMENT_CLASS(TEXTURE2D, Texture2D)
			ElEMENT_CLASS(NORMAL, Normal)
			ElEMENT_CLASS(TANGENT, Tangent)
			ElEMENT_CLASS(BIT_TANGENT, BitTangent)
			ElEMENT_CLASS(FLOAT3_COLOR, Float3Color)
			ElEMENT_CLASS(FLOAT4_COLOR, Float4Color)
#undef ElEMENT_CLASS
	};


	template<typename T> struct Element;

	template<> struct Element<VertexElement::POSITION2>
	{
		static constexpr GLenum type = GL_FLOAT;
		static constexpr GLint size = 2;
		static constexpr const char* semantic = "Position";
	};
	template<> struct Element<VertexElement::POSITION3>
	{
		static constexpr GLenum type = GL_FLOAT;
		static constexpr GLint size = 3;
		static constexpr const char* semantic = "Position";
	};
	template<> struct Element<VertexElement::TEXTURE2D>
	{
		static constexpr GLenum type = GL_FLOAT;
		static constexpr GLint size = 2;
		static constexpr const char* semantic = "Texcoord";
	};
	template<> struct Element<VertexElement::NORMAL>
	{
		static constexpr GLenum type = GL_FLOAT;
		static constexpr GLint size = 3;
		static constexpr const char* semantic = "Normal";
	};
	template<> struct Element<VertexElement::TANGENT>
	{
		static constexpr GLenum type = GL_FLOAT;
		static constexpr GLint size = 3;
		static constexpr const char* semantic = "Tagent";
	};
	template<> struct Element<VertexElement::BIT_TANGENT>
	{
		static constexpr GLenum type = GL_FLOAT;
		static constexpr GLint size = 3;
		static constexpr const char* semantic = "BitTagent";
	};
	template<> struct Element<VertexElement::FLOAT3_COLOR>
	{
		static constexpr GLenum type = GL_FLOAT;
		static constexpr GLint size = 3;
		static constexpr const char* semantic = "Color";
	};
	template<> struct Element<VertexElement::FLOAT4_COLOR>
	{
		static constexpr GLenum type = GL_FLOAT;
		static constexpr GLint size = 4;
		static constexpr const char* semantic = "Color";
	};

	class VertexElementParser
	{
	public:
#define VertexElementParser_SWITCH_CASE(name) \
	case VertexElement::TYPE::name: \
		m_Type = Element<VertexElement::name>::type;	\
		mSize = Element<VertexElement::name>::size;	\
		mSemantic = Element<VertexElement::name>::semantic; \
		break;

		VertexElementParser(VertexElement::TYPE type)
		{
			switch (type)
			{
				VertexElementParser_SWITCH_CASE(POSITION2)
				VertexElementParser_SWITCH_CASE(POSITION3)
				VertexElementParser_SWITCH_CASE(TEXTURE2D)
				VertexElementParser_SWITCH_CASE(NORMAL)
				VertexElementParser_SWITCH_CASE(TANGENT)
				VertexElementParser_SWITCH_CASE(BIT_TANGENT)
				VertexElementParser_SWITCH_CASE(FLOAT3_COLOR)
				VertexElementParser_SWITCH_CASE(FLOAT4_COLOR)
			}
		}
#undef VertexElementParser_SWITCH_CASE

		GLenum GetElementType() { return m_Type; };
		GLint GetSize() { return mSize; };
		const char* GetSemantic() { return mSemantic; };
	private:
		GLenum m_Type;
		GLint mSize;
		const char* mSemantic;
	};

	typedef struct INPUT_ELEMENT_DESC
	{
		GLenum type;
		GLint count;
		bool normlized = GL_FALSE;

		constexpr static unsigned int GetSizeOfType(unsigned int type)
		{
			switch (type)
			{
			case GL_FLOAT:			return 4;
			case GL_UNSIGNED_INT:	return 4;
			case GL_UNSIGNED_BYTE:	return 1;
			}

			return 0;
		}
	} OPENGL_INPUT_ELEMENT_DESC;

	class InputLayout
	{
	public:
		template<typename T>
		void Push(unsigned int count)
		{
			static_assert(false);
		}
		template<>
		void Push<float>(unsigned int count)
		{
			count = int(count);
			m_Elements.push_back({ GL_FLOAT,static_cast<GLint>(count),GL_FALSE });
			m_Stride += OPENGL_INPUT_ELEMENT_DESC::GetSizeOfType(GL_FLOAT) * count;
		}
		template<>
		void Push<unsigned int>(unsigned int count)
		{
			m_Elements.push_back({ GL_UNSIGNED_INT,static_cast<GLint>(count),GL_FALSE });
			m_Stride += OPENGL_INPUT_ELEMENT_DESC::GetSizeOfType(GL_UNSIGNED_INT) * count;
		}
		template<>
		void Push<unsigned char>(unsigned int count)
		{
			m_Elements.push_back({ GL_UNSIGNED_BYTE,static_cast<GLint>(count),GL_TRUE });
			m_Stride += OPENGL_INPUT_ELEMENT_DESC::GetSizeOfType(GL_UNSIGNED_BYTE) * count;
		}

		inline const std::vector<OPENGL_INPUT_ELEMENT_DESC> GetElements() const { return m_Elements; }
		inline unsigned int GetStride() const { return m_Stride; }
	protected:
		unsigned int m_Stride;
		unsigned int index;
		std::vector<OPENGL_INPUT_ELEMENT_DESC> m_Elements;
	};

VISIONGAL_OPENGL_NAMESPACE_END
