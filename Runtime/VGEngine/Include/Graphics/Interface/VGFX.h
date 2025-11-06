#pragma once
#include "Texture.h"
#include "Device.h"
#include "Mesh.h"
#include "Shader.h"
#include "VertexElement.h"
#include "../../EngineConfig.h"
#include "../../Core/Core.h"



namespace VisionGal::VGFX
{
	enum VGFX_SHADER_TYPE
	{
		VERTEX_SHADER,
		FRAGMENT_SHADER
	};

	void EnableDepthTest(bool enable);

	Ref<IShader> CreateShaderBySource(int shaderType, const String& source);
	Ref<IShaderProgram> CreateProgram(const std::vector<IShader*>& shaders);
	 
	void UseProgram(IShaderProgram* program);
	void SetUniformInt(const std::string& name, int v);
	void SetUniformInt2(const std::string& name, int2 v);
	void SetUniformInt3(const std::string& name, int3 v);
	void SetUniformInt4(const std::string& name, int4 v);
	void SetUniformBool(const std::string& name, bool v);
	void SetUniformFloat(const std::string& name, float v);
	void SetUniformFloat2(const std::string& name, const Horizon::float2& v);
	void SetUniformFloat3(const std::string& name, const Horizon::float3& v3);
	void SetUniformFloat4(const std::string& name, const Horizon::float4& v4);
	void SetUniformMatrix2(const std::string& name, const Horizon::matrix2x2& matrix);
	void SetUniformMatrix3(const std::string& name, const Horizon::matrix3x3& matrix);
	void SetUniformMatrix4(const std::string& name, const Horizon::matrix4x4& matrix);

	bool BindTexture(uint32_t slot, ITexture* texture = nullptr);
	bool SetTexture(uint32_t slot, const std::string& name, ITexture* texture = nullptr);
	bool SetTexture2DNative(uint32_t slot, const std::string& name, unsigned int renderID);

	void SetViewRect(int x, int y, unsigned int width, unsigned int height);

	Ref<IStaticMesh> CreateStaticMesh(void* vertexData, unsigned int vertexSize, void* indexData, unsigned int indexSize,const std::vector<VertexElement::IElement*>& elements);
	template<class VERTEX, class INDEX>
	Ref<IStaticMesh> CreateStaticMesh(std::vector<VERTEX>& vertices,std::vector<INDEX>& indices, const std::vector<VertexElement::IElement*>& elements)
	{
		void* vertexData = vertices.data();
		unsigned int vertexSize = vertices.size() * sizeof(VERTEX);
		void* indexData = indices.data();
		unsigned int indexSize = indices.size() * sizeof(INDEX);

		return CreateStaticMesh(vertexData, vertexSize, indexData, indexSize, elements);
	}
	void RenderMesh(IStaticMesh* mesh);
}