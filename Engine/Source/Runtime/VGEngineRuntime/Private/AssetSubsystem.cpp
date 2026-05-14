#include "VGEngineRuntime/AssetSubsystem.h"

namespace visiongal::engine
{
VGTextureHandle AssetSubsystem::LoadTexture(const char* virtualPathUtf8) noexcept
{
	(void)virtualPathUtf8;
	return 0;
}

VGAudioHandle AssetSubsystem::LoadAudio(const char* virtualPathUtf8) noexcept
{
	(void)virtualPathUtf8;
	return 0;
}
} // namespace visiongal::engine
