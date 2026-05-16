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
#include "Texture.h"
#include "Device.h"
#include "Mesh.h"
#include "Shader.h"
#include "VertexElement.h"
#include "../Include/VGRHIConfig.h"
#include <HCore/Interface/HCore.h>
#include <HCore/Interface/HCoreTypes.h>

namespace VisionGal::VGFX
{
	enum VGFX_SHADER_TYPE
	{
		VERTEX_SHADER,
		FRAGMENT_SHADER
	};

	VG_RHI_API void EnableDepthTest(bool enable);

	VG_RHI_API Ref<IShader> CreateShaderBySource(int shaderType, const std::string& source);
	VG_RHI_API Ref<IShaderProgram> CreateProgram(const std::vector<IShader*>& shaders);
	 
	VG_RHI_API void UseProgram(IShaderProgram* program);
	VG_RHI_API void SetUniformInt(const std::string& name, int v);
	VG_RHI_API void SetUniformInt2(const std::string& name, Horizon::int2 v);
	VG_RHI_API void SetUniformInt3(const std::string& name, Horizon::int3 v);
	VG_RHI_API void SetUniformInt4(const std::string& name, Horizon::int4 v);
	VG_RHI_API void SetUniformBool(const std::string& name, bool v);
	VG_RHI_API void SetUniformFloat(const std::string& name, float v);
	VG_RHI_API void SetUniformFloat2(const std::string& name, const Horizon::float2& v);
	VG_RHI_API void SetUniformFloat3(const std::string& name, const Horizon::float3& v3);
	VG_RHI_API void SetUniformFloat4(const std::string& name, const Horizon::float4& v4);
	VG_RHI_API void SetUniformMatrix2(const std::string& name, const Horizon::matrix2x2& matrix);
	VG_RHI_API void SetUniformMatrix3(const std::string& name, const Horizon::matrix3x3& matrix);
	VG_RHI_API void SetUniformMatrix4(const std::string& name, const Horizon::matrix4x4& matrix);

	VG_RHI_API bool BindTexture(uint32_t slot, ITexture* texture = nullptr);
	VG_RHI_API bool SetTexture(uint32_t slot, const std::string& name, ITexture* texture = nullptr);
	VG_RHI_API bool SetTexture2DNative(uint32_t slot, const std::string& name, unsigned int renderID);

	VG_RHI_API void SetViewRect(int x, int y, unsigned int width, unsigned int height);

	VG_RHI_API Ref<IStaticMesh> CreateStaticMesh(void* vertexData, unsigned int vertexSize, void* indexData, unsigned int indexSize,const std::vector<VertexElement::IElement*>& elements);
	template<class VERTEX, class INDEX>
	Ref<IStaticMesh> CreateStaticMesh(std::vector<VERTEX>& vertices,std::vector<INDEX>& indices, const std::vector<VertexElement::IElement*>& elements)
	{
		void* vertexData = vertices.data();
		unsigned int vertexSize = vertices.size() * sizeof(VERTEX);
		void* indexData = indices.data();
		unsigned int indexSize = indices.size() * sizeof(INDEX);

		return CreateStaticMesh(vertexData, vertexSize, indexData, indexSize, elements);
	}
	VG_RHI_API void RenderMesh(IStaticMesh* mesh);
}