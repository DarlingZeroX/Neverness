#include "pch.h"
#include "ImGuiEx/ImGuiImage.h"

namespace ImGuiEx
{
	ImGuiExImageManager* s_ImGuiExImageManager;

	std::shared_ptr<ImImage> FindImageID(const std::string& path)
	{
		return s_ImGuiExImageManager->GetImageID(path);
	}

	void SetImGuiExImageManager(ImGuiExImageManager* manager)
	{
		s_ImGuiExImageManager = manager;
	}
}