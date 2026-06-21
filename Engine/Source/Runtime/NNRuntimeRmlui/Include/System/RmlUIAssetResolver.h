#pragma once

/**
 * @file RmlUIAssetResolver.h
 * @brief RmlUI 资产路径解析器接口——替代 NNRuntimeScene::IAssetResolver。
 *
 * 由 ViewportRenderRuntimeApi 实现，注入 RmlUIRenderer。
 */

#include <cstdint>
#include "NNNativeEngineAPI/Include/EngineTypes.h"

namespace NN::Runtime::RmlUI
{
	/// @brief 资产路径解析器接口（原 NN::Runtime::Scene::IAssetResolver）。
	class IRmlUIAssetResolver
	{
	public:
		virtual ~IRmlUIAssetResolver() = default;

		/// @brief 根据 GUID 解析资产路径。
		/// @param guid 资产 GUID
		/// @param outPath 输出路径缓冲区
		/// @param outPathSize 缓冲区大小
		/// @return true 成功，false 失败
		virtual bool Resolve(NNGuid guid, char* outPath, std::uint32_t outPathSize) noexcept = 0;
	};

} // namespace NN::Runtime::RmlUI
