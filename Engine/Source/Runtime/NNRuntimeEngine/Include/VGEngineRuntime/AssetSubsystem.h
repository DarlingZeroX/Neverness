#pragma once

#include "NNNativeEngineAPI/EngineHandles.h"

namespace visiongal::engine
{
/**
 * @brief 資源子系統空殼：擴充載入 API 未接 **VGAsset** 前回傳 0 Handle。
 */
class AssetSubsystem final
{
public:
	VGTextureHandle LoadTexture(const char* virtualPathUtf8) noexcept;
	VGAudioHandle LoadAudio(const char* virtualPathUtf8) noexcept;
};
} // namespace visiongal::engine
