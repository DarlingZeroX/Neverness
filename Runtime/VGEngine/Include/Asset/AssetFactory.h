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
#include "Asset.h"
#include "../EngineConfig.h"
#include "../Interface/AssetInterface.h"

namespace VisionGal
{
    /// <summary>
    /// 引擎所有资产的创建工厂
    /// </summary>
    class VG_ENGINE_API EngineAssetFactory : public IEngineAssetFactory
    {
    public:
        EngineAssetFactory();
        ~EngineAssetFactory() override = default;
        EngineAssetFactory(const EngineAssetFactory&) = delete;
        EngineAssetFactory& operator=(const EngineAssetFactory&) = delete;
        EngineAssetFactory(EngineAssetFactory&&) noexcept = default;
        EngineAssetFactory& operator=(EngineAssetFactory&&) noexcept = default;

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
        SceneAssetFactory(const SceneAssetFactory&) = default;
        SceneAssetFactory& operator=(const SceneAssetFactory&) = default;
        SceneAssetFactory(SceneAssetFactory&&) noexcept = default;
        SceneAssetFactory& operator=(SceneAssetFactory&&) noexcept = default;
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
        UIDocumentAssetFactory(const UIDocumentAssetFactory&) = default;
        UIDocumentAssetFactory& operator=(const UIDocumentAssetFactory&) = default;
        UIDocumentAssetFactory(UIDocumentAssetFactory&&) noexcept = default;
        UIDocumentAssetFactory& operator=(UIDocumentAssetFactory&&) noexcept = default;
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
        UICssAssetFactory(const UICssAssetFactory&) = default;
        UICssAssetFactory& operator=(const UICssAssetFactory&) = default;
        UICssAssetFactory(UICssAssetFactory&&) noexcept = default;
        UICssAssetFactory& operator=(UICssAssetFactory&&) noexcept = default;
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
        LuaScriptAssetFactory(const LuaScriptAssetFactory&) = default;
        LuaScriptAssetFactory& operator=(const LuaScriptAssetFactory&) = default;
        LuaScriptAssetFactory(LuaScriptAssetFactory&&) noexcept = default;
        LuaScriptAssetFactory& operator=(LuaScriptAssetFactory&&) noexcept = default;
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
        GalGameStoryScriptFactory(const GalGameStoryScriptFactory&) = default;
        GalGameStoryScriptFactory& operator=(const GalGameStoryScriptFactory&) = default;
        GalGameStoryScriptFactory(GalGameStoryScriptFactory&&) noexcept = default;
        GalGameStoryScriptFactory& operator=(GalGameStoryScriptFactory&&) noexcept = default;
        ~GalGameStoryScriptFactory() override = default;

        std::string GetFactoryType() override;
        Ref<VGAsset> CreateAsset(const String& path) override;
    };

    VG_ENGINE_API EngineAssetFactory* GetEngineAssetFactory();
}
