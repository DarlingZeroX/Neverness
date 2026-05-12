/*
 * GalGameLayoutUtils 实现
 */

#include "GalGameLayoutUtils.h"

#include "VGEngine/Include/Project/ProjectSettings.h"

namespace VisionGal::GalGame
{
	void GalGameLayoutUtils::SetDesignSize(float2 size)
	{
		auto& settings = ProjectSettings::GetProjectSettings().GalGame;
		settings.DesignWidth = static_cast<int>(size.x);
		settings.DesignHeight = static_cast<int>(size.y);
	}

	float2 GalGameLayoutUtils::GetDesignSize()
	{
		float2 size;
		auto& settings = ProjectSettings::GetProjectSettings().GalGame;
		size.x = static_cast<float>(settings.DesignWidth);
		size.y = static_cast<float>(settings.DesignHeight);
		return size;
	}

	float GalGameLayoutUtils::GetSpriteYOffset(float size_y)
	{
		return -(GetDesignSize().y - size_y) / 2.0f;
	}

	float GalGameLayoutUtils::GetSpriteXOffset(float size_x)
	{
		return -(GetDesignSize().x - size_x) / 2.0f;
	}
}
