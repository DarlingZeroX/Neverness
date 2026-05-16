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
#include "../../Interface/Shader.h"
#include "Interface.h"
#include "Shader.h"
#include <vector>
#include <unordered_map>
#include <HCore/Interface/HCoreTypes.h>

VISIONGAL_OPENGL_NAMESPACE_BEGIN

	class VG_RHI_API ShaderProgram :public VGFX::IShaderProgram,public Bindable
	{
	public:
		ShaderProgram() = default;
		ShaderProgram(ShaderProgram&) = delete;
		ShaderProgram& operator=(ShaderProgram&) = delete;
		~ShaderProgram() override = default;

		ShaderProgram(const std::vector<Shader*>& shaderes);
	public:
		static Ref<ShaderProgram> Create(const std::vector<Shader*>& shaderes);
		static Ref<ShaderProgram> Create(const std::vector<VGFX::IShader*>& shaderes);

		bool Exist() const;
		void DetachShader(GLuint shader);
		virtual void Bind() const override;
		virtual void Unbind() const override;

		std::string GetErrorMsg() const;
	public: 
		void UseProgram() override;
		unsigned int GetRenderID() { return m_RendererID; }

		void SetInt(const std::string& name, int v) override;
		void SetInt2(const std::string& name, Horizon::int2 v) override;
		void SetInt3(const std::string& name, Horizon::int3 v) override;
		void SetInt4(const std::string& name, Horizon::int4 v) override;
		void SetBool(const std::string& name, bool v) override;
		void SetFloat(const std::string& name, float v) override;
		void SetFloat2(const std::string& name, const Horizon::float2& v) override;
		void SetFloat3(const std::string& name, const Horizon::float3& v3) override;
		void SetFloat4(const std::string& name, const Horizon::float4& v4) override;
		void SetMatrix2(const std::string& name, const Horizon::matrix2x2& matrix) override;
		void SetMatrix3(const std::string& name, const Horizon::matrix3x3& matrix) override;
		void SetMatrix4(const std::string& name, const Horizon::matrix4x4& matrix) override;
	private:
		bool CreateImp(const std::vector<VGFX::IShader*>& shaderes);
		bool CreateImp(const std::vector<Shader*>& shaderes);

		int GetUniformLocation(const std::string& name);

		void Clear();
		void CreateProgram();
		void LinkProgram() const;
		void ValidateProgram() const;
		void UseProgram() const;
		void AttachShader(VGFX::IShader* shader) const;
		void DeleteProgram();
		void GetProgramiv(GLenum pname, GLint* param);
		void GetProgramInfoLog(GLsizei bufSize, GLsizei* length, GLchar* infoLog);
	private:
		std::string m_ErrorMsg;
		unsigned int m_RendererID = 0;
		std::unordered_map < std::string, int> m_UniformLocationCache;
	};

VISIONGAL_OPENGL_NAMESPACE_END
