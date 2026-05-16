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

#include "Engine/Manager/ShaderManager.h"
#include "NNRuntimeCore/Include/Core/VFS.h"

namespace VisionGal
{
	ShaderManager::ShaderManager()
	{
		Initialize();
	}

	ShaderManager* ShaderManager::Get()
	{
		static ShaderManager manager;

		return &manager;
	}

	void ShaderManager::AddBuiltInShader(const std::string& name, const std::string& path)
	{
		std::string shaderPath = Core::GetEngineResourcePathVFS() + "shaders/";

		if (path.empty())
			shaderPath += name + ".glsl";
		else
			shaderPath += path;

		std::string shader;
		if (VFS::ReadTextFromFile(shaderPath, shader))
		{
			std::string vertexShader, fragmentShader;
			ExtractShaders(shader, vertexShader, fragmentShader);

			auto vs = VGFX::CreateShaderBySource(VGFX::VERTEX_SHADER, vertexShader);
			auto ps = VGFX::CreateShaderBySource(VGFX::FRAGMENT_SHADER, fragmentShader);
			m_BuiltinProgram[name] = VGFX::CreateProgram({ vs.get(), ps.get() });
		}
		else
		{
			H_LOG_ERROR("Failed to load built-in shader: " + name);
		}
	}

	void ShaderManager::Initialize()
	{
		AddBuiltInShader("TransitionFade", "transition/TransitionFade.glsl");
		AddBuiltInShader("TransitionPush", "transition/TransitionPush.glsl");
		AddBuiltInShader("TransitionCustomImage", "transition/TransitionCustomImage.glsl");

		AddBuiltInShader("SpriteDefault");
		AddBuiltInShader("FullScreenQuad");
		AddBuiltInShader("ViewportOutput");
		AddBuiltInShader("MixCharacterSprite");
	}

	VGFX::IShaderProgram* ShaderManager::GetBuiltinProgram(const String& name)
	{
		auto iterator = m_BuiltinProgram.find(name);
		if (iterator != m_BuiltinProgram.end())
		{
			return iterator->second.get();
		}
	}

	std::string ShaderManager::Trim(const std::string& s)
	{
		size_t start = s.find_first_not_of(" \t\r\n");
		size_t end = s.find_last_not_of(" \t\r\n");
		return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
	}

	void ShaderManager::ExtractShaders(const std::string& source, std::string& vs, std::string& ps)
	{
		std::istringstream stream(source);
		std::string line;
		ShaderBlockType current = ShaderBlockType::None;
		std::ostringstream vsStream, psStream;

		int braceDepth = 0;
		bool insideBlock = false;

		while (std::getline(stream, line)) {
			std::string trimmed = Trim(line);

			if (current == ShaderBlockType::None) {
				if (trimmed == "VS{") {
					current = ShaderBlockType::VS;
					braceDepth = 1;
					insideBlock = true;
					continue;
				}
				else if (trimmed == "PS{") {
					current = ShaderBlockType::PS;
					braceDepth = 1;
					insideBlock = true;
					continue;
				}
			}
			else if (insideBlock) {
				// Count braces to determine block end
				for (char c : line) {
					if (c == '{') braceDepth++;
					else if (c == '}') braceDepth--;
				}

				if (braceDepth == 0) {
					current = ShaderBlockType::None;
					insideBlock = false;
					continue;
				}

				if (current == ShaderBlockType::VS)
					vsStream << line << '\n';
				else if (current == ShaderBlockType::PS)
					psStream << line << '\n';
			}
		}

		vs = vsStream.str();
		ps = psStream.str();
	}
}
