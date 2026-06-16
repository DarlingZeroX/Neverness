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

#include "Render/Material.h"
#include "Engine/Manager.h"

namespace NN::Runtime
{
    // 顶点着色器
    const char* s_DefualtVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;

		uniform mat4 model;
		uniform mat4 projection;
        
        void main() {
            //gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);

            TexCoord = aTexCoord;
			gl_Position = projection * model * vec4(aPos, 1.0);

            //gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        }
    )";
	 
    // 片段着色器
    const char* s_DefualtFragmentShader = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        
        uniform sampler2D texture1;
        
        void main() {
            FragColor = texture(texture1, TexCoord);
			//FragColor = vec4(1.0f);
        }
    )";

	Material::Material()
	{
        //auto vs = OpenGL::CreateShader(GL_VERTEX_SHADER, s_DefualtVertexShader);
        //auto ps = OpenGL::CreateShader(GL_FRAGMENT_SHADER, s_DefualtFragmentShader);
        //std::vector<OpenGL::Shader*> shaders;
        //shaders.push_back(vs.get());
        //shaders.push_back(ps.get());
        //m_ShaderProgram = new OpenGL::ShaderProgram(shaders);

        m_ShaderProgram = ShaderManager::Get()->GetBuiltinProgram("SpriteDefault");
	}

	Ref<Material> Material::Create()
	{
		return MakeRef<Material>();
	}

    VGFX::IShaderProgram* Material::GetShaderProgram()
	{
        return m_ShaderProgram;
        //if(m_ShaderProgram)
		//return m_ShaderProgram.get();
	}
}
