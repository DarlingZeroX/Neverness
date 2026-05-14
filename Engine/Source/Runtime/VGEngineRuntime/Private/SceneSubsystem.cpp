#include "VGEngineRuntime/SceneSubsystem.h"

namespace visiongal::engine
{
int SceneSubsystem::LoadScene(const char* sceneNameUtf8) noexcept
{
	(void)sceneNameUtf8;
	// 空殼：與 Stub 一致回傳「成功」整數，但不建立真實場景。
	return 1;
}

VGEntityHandle SceneSubsystem::Spawn(const char* prefabVirtualPathUtf8) noexcept
{
	(void)prefabVirtualPathUtf8;
	return 0;
}

void SceneSubsystem::Destroy(VGEntityHandle entity) noexcept
{
	(void)entity;
}

VGEntityHandle SceneSubsystem::Find(const char* entityNameUtf8) noexcept
{
	(void)entityNameUtf8;
	return 0;
}

void SceneSubsystem::Activate(VGEntityHandle entity, int active) noexcept
{
	(void)entity;
	(void)active;
}
} // namespace visiongal::engine
