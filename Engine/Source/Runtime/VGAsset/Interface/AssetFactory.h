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
#include "VGCore/Interface/VGAsset.h"
#include "../VGAssetConfig.h"
#include "VGCore/Interface/AssetInterface.h"

namespace VisionGal
{
    /// <summary>
    /// 引擎所有资产的创建工厂
    /// </summary>
    class VG_ASSET_API EngineAssetFactory : public IEngineAssetFactory
    {
		EngineAssetFactory();
    public:
        ~EngineAssetFactory() override = default;
        EngineAssetFactory(const EngineAssetFactory&) = delete;
        EngineAssetFactory& operator=(const EngineAssetFactory&) = delete;
        EngineAssetFactory(EngineAssetFactory&&) noexcept = default;
        EngineAssetFactory& operator=(EngineAssetFactory&&) noexcept = default;

    	static EngineAssetFactory& Get();
		void RegisterFactory(Scope<IAssetFactoryInstance> factoryInstance);
        Ref<VGAsset> CreateAsset(const String& path, const String& type) override;
    private:
        std::vector<Scope<IAssetFactoryInstance>> m_AssetFactoryInstances;
    };
	 
    /// <summary>
    /// 场景资产创建工厂
    /// </summary>
    struct SceneAssetFactory: public IAssetFactoryInstance
    {
        SceneAssetFactory() = default;
    	~SceneAssetFactory() override = default;

        std::string GetFactoryType() override;
    	Ref<VGAsset> CreateAsset(const String& path) override;
    };

    /// <summary>
    /// UI文档资产创建工厂
    /// </summary>
    struct UIDocumentAssetFactory : public IAssetFactoryInstance
    {
        UIDocumentAssetFactory() = default;
        ~UIDocumentAssetFactory() override = default;

        std::string GetFactoryType() override;
        Ref<VGAsset> CreateAsset(const String& path) override;
    };

    /// <summary>
	/// UI样式表资产创建工厂
	/// </summary>
    struct UICssAssetFactory : public IAssetFactoryInstance
    {
        UICssAssetFactory() = default;
        ~UICssAssetFactory() override = default;

        std::string GetFactoryType() override;
        Ref<VGAsset> CreateAsset(const String& path) override;
    };

    /// <summary>
	/// UI样式表资产创建工厂
	/// </summary>
    struct LuaScriptAssetFactory : public IAssetFactoryInstance
    {
        LuaScriptAssetFactory() = default;
        ~LuaScriptAssetFactory() override = default;

        std::string GetFactoryType() override;
        Ref<VGAsset> CreateAsset(const String& path) override;
    };

    /// <summary>
	/// UI样式表资产创建工厂
	/// </summary>
    struct GalGameStoryScriptFactory : public IAssetFactoryInstance
    {
        GalGameStoryScriptFactory() = default;
        ~GalGameStoryScriptFactory() override = default;

        std::string GetFactoryType() override;
        Ref<VGAsset> CreateAsset(const String& path) override;
    };
}
