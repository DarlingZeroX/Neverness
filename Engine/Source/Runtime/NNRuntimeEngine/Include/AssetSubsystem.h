#pragma once

#include "NNNativeEngineAPI/Include/EngineHandles.h"

namespace NN::Runtime::engine
{
/**
 * @brief 資源子系統空殼：擴充載入 API 未接 **VGAsset** 前回傳 0 Handle。
 */
class AssetSubsystem final
{
public:
	NNTextureHandle LoadTexture(const char* virtualPathUtf8) noexcept;
	NNAudioHandle LoadAudio(const char* virtualPathUtf8) noexcept;
};
} // namespace visiongal::engine
