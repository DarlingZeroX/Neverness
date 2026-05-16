#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include "../imgui/imgui.h"

namespace ImGuiEx
{
	class IMGUI_API ImFontManager
	{
	public:
		ImFontAtlas* GetFont(std::string fontname, int size_pixels, std::string text);

		void RemoveFont(std::string fontname, int size_pixels, std::string text);

		void Update();

		static ImFontManager* GetInstance();
	private:
		struct ImportFontData
		{
			std::string fontname;
			int size_pixels;
			std::string text;
		};
		
		std::unordered_map < std::string, std::unordered_map<int, ImFontAtlas*>> m_FontKeyMap;
		std::vector<ImportFontData> m_ImportFontVector;
	};

}