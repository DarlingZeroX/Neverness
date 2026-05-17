/**
 * @file NNEntity.cpp
 * @brief NNEntity 打包/解包实现。
 */

#include "Scene/NNEntity.h"

namespace NN::Runtime::Scene
{
NNEntity PackEntityHandle(const std::uint32_t index, const std::uint32_t generation) noexcept
{
	return (static_cast<NNEntity>(generation) << 32u) | static_cast<NNEntity>(index);
}

NNEntityHandleParts UnpackEntityHandle(const NNEntity handle) noexcept
{
	NNEntityHandleParts parts{};
	parts.Index = static_cast<std::uint32_t>(handle & 0xFFFFFFFFu);
	parts.Generation = static_cast<std::uint32_t>(handle >> 32u);
	return parts;
}
} // namespace NN::Runtime::Scene
