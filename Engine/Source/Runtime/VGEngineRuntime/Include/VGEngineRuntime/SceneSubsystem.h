#pragma once

#include "VGNativeEngineAPI/EngineHandles.h"

namespace visiongal::engine
{
/**
 * @brief 場景子系統空殼：未接線 **VGEngine** 前一律回傳「無效」或 no-op。
 */
class SceneSubsystem final
{
public:
	int LoadScene(const char* sceneNameUtf8) noexcept;
	VGEntityHandle Spawn(const char* prefabVirtualPathUtf8) noexcept;
	void Destroy(VGEntityHandle entity) noexcept;
	VGEntityHandle Find(const char* entityNameUtf8) noexcept;
	void Activate(VGEntityHandle entity, int active) noexcept;
};
} // namespace visiongal::engine
