/*
* This source file is part of VisionGal, the Visual Novel Engine
*
* For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
* GitHub page: https://github.com/DarlingZeroX/VisionGal
*
* Copyright (c) 2025-present 梦旅缘心
*
* See the LICENSE file in the project root for details.
*
* Phase 7：IStoryScriptExecutor / IStoryScriptExecutorCreator 已迁至 VGGalgameCore；
* 本头文件保留 GalGameScriptExecutorFactory 并转发包含 Core 契约，供旧 include 路径兼容。
*/ 

#pragma once
#include "../VGGalgameRuntimeConfig.h"
#include "VGGalgameCore/Interface/IStoryScriptExecutor.h"
#include "VGCore/Include/Core/Core.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace VisionGal::GalGame
{
	struct VG_GALGAME_RUNTIME_API GalGameScriptExecutorFactory
	{
		virtual Ref<IStoryScriptExecutor> LoadAssetExecutor(const String& type, const String& path);

		virtual void RegisterAssetExecutor(const String& type, Ref<IStoryScriptExecutorCreator> factoryFunction);

		static GalGameScriptExecutorFactory& Get();

		std::vector<std::string> GetRegisterTypes();

		bool HasExecutor(const String& type);
	private:
		std::unordered_map<String, Ref<IStoryScriptExecutorCreator>> m_FactoryFunctions;
	};

}
