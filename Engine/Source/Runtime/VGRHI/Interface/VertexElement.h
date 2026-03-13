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
#include <HCore/Include/Core/HCoreTypes.h>

namespace VisionGal::VGFX
{
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
#define VGFX_ElEMENT_CLASS(class_name,entity_name)			\
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

		VGFX_ElEMENT_CLASS(POSITION2, Position2)
		VGFX_ElEMENT_CLASS(POSITION3, Position3)
		VGFX_ElEMENT_CLASS(TEXTURE2D, Texture2D)
		VGFX_ElEMENT_CLASS(NORMAL, Normal)
		VGFX_ElEMENT_CLASS(TANGENT, Tangent)
		VGFX_ElEMENT_CLASS(BIT_TANGENT, BitTangent)
		VGFX_ElEMENT_CLASS(FLOAT3_COLOR, Float3Color)
		VGFX_ElEMENT_CLASS(FLOAT4_COLOR, Float4Color)
#undef VGFX_ElEMENT_CLASS
	};

	enum class ElemnetType
	{
		FLOAT
	};

	template<typename T> struct Element;

	template<> struct Element<VertexElement::POSITION2>
	{
		static constexpr ElemnetType type = ElemnetType::FLOAT;
		static constexpr int size = 2;
		static constexpr const char* semantic = "Position";
	};
	template<> struct Element<VertexElement::POSITION3>
	{
		static constexpr ElemnetType type = ElemnetType::FLOAT;
		static constexpr int size = 3;
		static constexpr const char* semantic = "Position";
	};
	template<> struct Element<VertexElement::TEXTURE2D>
	{
		static constexpr ElemnetType type = ElemnetType::FLOAT;
		static constexpr int size = 2;
		static constexpr const char* semantic = "Texcoord";
	};
	template<> struct Element<VertexElement::NORMAL>
	{
		static constexpr ElemnetType type = ElemnetType::FLOAT;
		static constexpr int size = 3;
		static constexpr const char* semantic = "Normal";
	};
	template<> struct Element<VertexElement::TANGENT>
	{
		static constexpr ElemnetType type = ElemnetType::FLOAT;
		static constexpr int size = 3;
		static constexpr const char* semantic = "Tagent";
	};
	template<> struct Element<VertexElement::BIT_TANGENT>
	{
		static constexpr ElemnetType type = ElemnetType::FLOAT;
		static constexpr int size = 3;
		static constexpr const char* semantic = "BitTagent";
	};
	template<> struct Element<VertexElement::FLOAT3_COLOR>
	{
		static constexpr ElemnetType type = ElemnetType::FLOAT;
		static constexpr int size = 3;
		static constexpr const char* semantic = "Color";
	};
	template<> struct Element<VertexElement::FLOAT4_COLOR>
	{
		static constexpr ElemnetType type = ElemnetType::FLOAT;
		static constexpr int size = 4;
		static constexpr const char* semantic = "Color";
	};

}