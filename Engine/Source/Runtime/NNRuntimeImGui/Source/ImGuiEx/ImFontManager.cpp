#include "ImGuiEx/ImFontManager.h"
#include "imgui/imgui_impl_sdlrenderer3.h"
#include "imgui/imgui_impl_sdl3.h"
#include <SDL3/SDL.h>

#include <SDL3/SDL_opengl.h>

#include <locale>
#include <codecvt>
#include <algorithm>

namespace ImGuiEx
{
	ImFontAtlas* ImFontManager::GetFont(std::string fontname, int size_pixels, std::string text)
	{
		if (text == "")
			return nullptr;

		if (size_pixels <= 0)
			return nullptr;

		// FontName
		if (m_FontKeyMap.find(fontname) != m_FontKeyMap.end())
		{
			auto& sizeFontMap = m_FontKeyMap[fontname];

			// Size_Pixels
			if (sizeFontMap.find(size_pixels) != sizeFontMap.end())
			{
				return sizeFontMap[size_pixels];
				//auto& fontMap = sizeFontMap[size_pixels];
				//
				//// text
				//if (fontMap.find(text) != fontMap.end())
				//{
				//	return fontMap[text];
				//
				//}
				//else
				//{
				//	m_ImportFontVector.push_back({ fontname ,size_pixels, text });
				//}
			}
			else
			{
				m_ImportFontVector.push_back({ fontname ,size_pixels, text });
			}
		}
		else
		{
			m_ImportFontVector.push_back({ fontname ,size_pixels, text });
		}

		return nullptr;
	}

	void ImFontManager::RemoveFont(std::string fontname, int size_pixels, std::string text)
	{
	}

	std::wstring StringToWString(const std::string& str) {
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring wideStr = converter.from_bytes(str);
		return wideStr; // 加上字符串结束符
	}

	// 将 std::wstring 转换为 ImWchar 数组
	void WstringToImWchar(const std::wstring& wstr, ImWchar* out) {
		size_t length = wstr.length();
		for (size_t i = 0; i < length; ++i) {
			out[i] = static_cast<ImWchar>(wstr[i]);
		}
		out[length] = 0; // 添加字符串结束符
	}

	void SortImWcharArray(ImWchar* arr) {
		// 找出有效字符范围
		ImWchar* end = arr;
		while (*end != 0) {
			++end;
		}

		// 对有效字符范围进行排序
		std::sort(arr, end);
	}

	void ImFontManager::Update()
	{
		ImGuiIO& io = ImGui::GetIO();
		ImFontAtlas* atlas = nullptr;

		if (m_ImportFontVector.size() > 0)
		{
			atlas = new ImFontAtlas();
		}

		for (size_t i = 0; i < m_ImportFontVector.size(); ++i) {
			auto& fontImportData = m_ImportFontVector[i];

			//auto wText = StringToWString(fontImportData.text);
			//
			//const size_t bufferSize = wText.length() + 1; // 加上字符串结束符的空间
			//ImWchar* output = new ImWchar[bufferSize];
			//
			//WstringToImWchar(wText, output);
			//
			//if(bufferSize != 2)
			//	SortImWcharArray(output);

			ImFontConfig fontConfig;
			// 确保字体数据由图集拥有
			fontConfig.FontDataOwnedByAtlas = true;
			fontConfig.SizePixels = fontImportData.size_pixels;
		
			auto* font = atlas->AddFontFromFileTTF(fontImportData.fontname.c_str(), fontImportData.size_pixels, &fontConfig, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

			m_FontKeyMap[fontImportData.fontname][fontImportData.size_pixels] = atlas;

			//delete[] output;
		}

		if (m_ImportFontVector.size() > 0)
		{
			//io.Fonts->AddFontDefault();
			atlas->Build();

			// 获取字体纹理数据
			unsigned char* pixels;
			int width, height;
			atlas->GetTexDataAsRGBA32(&pixels, &width, &height);

			GLuint fontTextureID = 0;

			// 删除旧的纹理
			if (fontTextureID != 0) {
				glDeleteTextures(1, &fontTextureID);
			}

			// 创建新的纹理
			glGenTextures(1, &fontTextureID);
			glBindTexture(GL_TEXTURE_2D, fontTextureID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

			// 更新 ImGui 的字体纹理句柄
			atlas->TexID = (ImTextureID)(intptr_t)fontTextureID;
		}

		m_ImportFontVector.clear();
	}

	ImFontManager* ImFontManager::GetInstance()
	{
		static ImFontManager* m_ImFontManagerInstance = nullptr;

		if (m_ImFontManagerInstance == nullptr)
			m_ImFontManagerInstance = new ImFontManager();

		return m_ImFontManagerInstance;
	}
}