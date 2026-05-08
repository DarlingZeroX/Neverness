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
#include "../VGCoreConfig.h"
#include "VGAsset.h"
#include "../Include/Core/Core.h"

namespace VisionGal
{
    struct IEngineAssetFactory
    {
        virtual ~IEngineAssetFactory() = default;

        virtual Ref<VGAsset> CreateAsset(const String& path, const String& type) = 0;
    };

    struct IAssetFactoryInstance
    {
        virtual ~IAssetFactoryInstance() = default;

        virtual std::string GetFactoryType() = 0;
        virtual Ref<VGAsset> CreateAsset(const String& path) = 0;
    };

	VG_CORE_API VGPath GenerateAssetPath(const VGPath& path, const std::string& name, const std::string& ext);
}