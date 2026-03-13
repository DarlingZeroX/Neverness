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

#include "../../Interface/VGFX.h"
#include "OpenGL/Texture2D.h"
#include "OpenGL/Mesh.h"
#include "OpenGL/Shader.h"
#include "OpenGL/ShaderProgram.h"
#include "OpenGL/Effect.h"
#include "OpenGL/ThrowMarco.h"

namespace VisionGal::VGFX
{
	static IShaderProgram* s_CurrentProgram = nullptr;

	void EnableDepthTest(bool enable)
	{
		OpenGL::RasterStates::DepthTest(enable);
	}

	Ref<IShader> CreateShaderBySource(int shaderType, const std::string& source)
	{
		switch (shaderType)
		{
		case VERTEX_SHADER:
			return OpenGL::CreateShader(GL_VERTEX_SHADER, source);
		case FRAGMENT_SHADER:
			return OpenGL::CreateShader(GL_FRAGMENT_SHADER, source);
		}
	}
	 
	Ref<IShaderProgram> CreateProgram(const std::vector<IShader*>& shaders)
	{
		return OpenGL::ShaderProgram::Create(shaders);;
	}

	void UseProgram(IShaderProgram* program)
	{
		s_CurrentProgram = program;
		s_CurrentProgram->UseProgram();
	}

	void SetUniformInt(const std::string& name, int v)
	{
		s_CurrentProgram->SetInt(name, v);
	}

	void SetUniformInt2(const std::string& name, Horizon::int2 v)
	{
		s_CurrentProgram->SetInt2(name, v);
	}

	void SetUniformInt3(const std::string& name, Horizon::int3 v)
	{
		s_CurrentProgram->SetInt3(name, v);
	}

	void SetUniformInt4(const std::string& name, Horizon::int4 v)
	{
		s_CurrentProgram->SetInt4(name, v);
	}

	void SetUniformBool(const std::string& name, bool v)
	{
		s_CurrentProgram->SetBool(name, v);
	}

	void SetUniformFloat(const std::string& name, float v)
	{
		s_CurrentProgram->SetFloat(name, v);
	}

	void SetUniformFloat2(const std::string& name, const Horizon::float2& v)
	{
		s_CurrentProgram->SetFloat2(name, v);
	}

	void SetUniformFloat3(const std::string& name, const Horizon::float3& v3)
	{
		s_CurrentProgram->SetFloat3(name, v3);
	}

	void SetUniformFloat4(const std::string& name, const Horizon::float4& v4)
	{
		s_CurrentProgram->SetFloat4(name, v4);
	}

	void SetUniformMatrix2(const std::string& name, const Horizon::matrix2x2& matrix)
	{
		s_CurrentProgram->SetMatrix2(name, matrix);
	}

	void SetUniformMatrix3(const std::string& name, const Horizon::matrix3x3& matrix)
	{
		s_CurrentProgram->SetMatrix3(name, matrix);
	}

	void SetUniformMatrix4(const std::string& name, const Horizon::matrix4x4& matrix)
	{
		s_CurrentProgram->SetMatrix4(name, matrix);
	}

	bool BindTexture(uint32_t slot, ITexture* texture)
	{
		if (texture == nullptr)
		{
			// 恢复到纹理单元0
			glActiveTexture(GL_TEXTURE0);
			return true;
		}


		auto* texgl = dynamic_cast<OpenGL::Texture2D*>(texture);
		texgl->Bind(slot);

		return true;
	}

	bool SetTexture(uint32_t slot, const std::string& name, ITexture* texture)
	{
		BindTexture(slot, texture);
		SetUniformInt(name, slot);

		return true;
	}

	bool SetTexture2DNative(uint32_t slot, const std::string& name, unsigned int renderID)
	{
		GL_THROW_INFO(glActiveTexture(GL_TEXTURE0 + slot));
		GL_THROW_INFO(glBindTexture(GL_TEXTURE_2D, renderID));

		SetUniformInt(name, slot);

		return true;
	}

	void SetViewRect(int x, int y, unsigned int width, unsigned int height)
	{
		glViewport(x, y, width, height);
	}

	Ref<IStaticMesh> CreateStaticMesh(void* vertexData, unsigned int vertexSize, void* indexData,
	                                  unsigned int indexSize, const std::vector<VertexElement::IElement*>& elements)
	{
		return CreateRef<OpenGL::StaticMesh>(vertexData,vertexSize,indexData,indexSize, elements);
	}

	void RenderMesh(IStaticMesh* mesh)
	{
		mesh->Render();
	}
}

