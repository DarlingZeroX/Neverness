/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*/

#pragma once
#include "../VGAssetConfig.h"
#include "NNRuntimeCore/Interface/VGAsset.h"
#include "NNRuntimeAssetLegacy/Include/SceneAsset.h"
#include "NNRuntimeCore/Include/Core/RuntimeCore.h"

namespace NN::Runtime
{
    class VG_ASSET_API SceneAssetLoader : public IAssetLoader
    {
    public:
        SceneAssetLoader() = default;
        ~SceneAssetLoader() override = default;

        bool Read(const std::string path, Ref<VGAsset>& asset) override;
    };

    class VG_ASSET_API SceneAssetWriter : public IAssetWriter
    {
    public:
        SceneAssetWriter() = default;
        ~SceneAssetWriter() override = default;

        bool Write(const std::string path, VGAsset* asset) override;
    };
}
