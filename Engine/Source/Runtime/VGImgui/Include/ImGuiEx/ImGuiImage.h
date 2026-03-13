#pragma once
#include <string>
#include "../imgui/imgui.h"
#include <memory>

namespace ImGuiEx
{
	struct ImImage
	{
		ImTextureID ID;
		unsigned int Width;
		unsigned int Height;
		bool IsValid = false;
	};

	IMGUI_API struct ImGuiExImageManager
	{
		virtual std::shared_ptr<ImImage> GetImageID(const std::string& path) = 0;
	};

	IMGUI_API std::shared_ptr<ImImage> FindImageID(const std::string& path);

	IMGUI_API void SetImGuiExImageManager(ImGuiExImageManager* manager);
}
