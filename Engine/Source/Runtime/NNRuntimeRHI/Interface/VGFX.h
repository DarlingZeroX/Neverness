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

// ============================================================================
// ⚠️  DEPRECATED — 此模块已废弃，将在未来版本中删除
// 请使用 NNRuntimeRender 接口（Rendering/NNRuntimeRender）替代 VGFX 接口
// 迁移文档：Engine/Source/Plan/Runtime_Diligent_Migration_Plan.md
// ============================================================================

#pragma once
#include "Texture.h"
#include "Device.h"
#include "Mesh.h"
#include "Shader.h"
#include "VertexElement.h"
#include "../Include/VGRHIConfig.h"
#include <NNCore/Interface/HCore.h>
#include <NNCore/Interface/HCoreTypes.h>

namespace NN::Runtime::VGFX
{
	enum VGFX_SHADER_TYPE
	{
		VERTEX_SHADER,
		FRAGMENT_SHADER
	};

	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void EnableDepthTest(bool enable);

	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API NN::Ref<IShader> CreateShaderBySource(int shaderType, const std::string& source);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API NN::Ref<IShaderProgram> CreateProgram(const std::vector<IShader*>& shaders);

	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void UseProgram(IShaderProgram* program);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetUniformInt(const std::string& name, int v);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetUniformInt2(const std::string& name, NN::Core::int2 v);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetUniformInt3(const std::string& name, NN::Core::int3 v);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetUniformInt4(const std::string& name, NN::Core::int4 v);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetUniformBool(const std::string& name, bool v);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetUniformFloat(const std::string& name, float v);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetUniformFloat2(const std::string& name, const NN::Core::float2& v);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetUniformFloat3(const std::string& name, const NN::Core::float3& v3);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetUniformFloat4(const std::string& name, const NN::Core::float4& v4);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetUniformMatrix2(const std::string& name, const NN::Core::matrix2x2& matrix);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetUniformMatrix3(const std::string& name, const NN::Core::matrix3x3& matrix);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetUniformMatrix4(const std::string& name, const NN::Core::matrix4x4& matrix);

	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API bool BindTexture(uint32_t slot, ITexture* texture = nullptr);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API bool SetTexture(uint32_t slot, const std::string& name, ITexture* texture = nullptr);
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API bool SetTexture2DNative(uint32_t slot, const std::string& name, unsigned int renderID);

	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void SetViewRect(int x, int y, unsigned int width, unsigned int height);

	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API NN::Ref<IStaticMesh> CreateStaticMesh(void* vertexData, unsigned int vertexSize, void* indexData, unsigned int indexSize,const std::vector<VertexElement::IElement*>& elements);
	template<class VERTEX, class INDEX>
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	NN::Ref<IStaticMesh> CreateStaticMesh(std::vector<VERTEX>& vertices,std::vector<INDEX>& indices, const std::vector<VertexElement::IElement*>& elements)
	{
		void* vertexData = vertices.data();
		unsigned int vertexSize = vertices.size() * sizeof(VERTEX);
		void* indexData = indices.data();
		unsigned int indexSize = indices.size() * sizeof(INDEX);

		return CreateStaticMesh(vertexData, vertexSize, indexData, indexSize, elements);
	}
	[[deprecated("Use NNRuntimeRender interfaces instead")]]
	VG_RHI_API void RenderMesh(IStaticMesh* mesh);
}