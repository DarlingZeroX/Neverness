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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <HCore/Interface/HConfig.h>
#include "../GSSExport.h"
#include "VGSTypeDefine.h"

namespace VisionGal
{
	// Visual GalGame Script Sequence Component Interface
	// 可视化 GalGame 脚本序列组件接口
	struct IVGSSequenceComponent
	{
		virtual ~IVGSSequenceComponent() = default;

		virtual std::string GetTypeNameID() = 0;
		virtual Ref<IVGSSequenceComponent> Clone() = 0;

		unsigned SequenceIndex; // 所属序列 ID
	};

	// Template base class for sequence components, providing default implementations for type name ID and cloning
	// 序列组件的模板基类，提供类型名称 ID 和克隆的默认实现
	template<class T>
	struct TVGSSequenceComponent:public IVGSSequenceComponent
	{
		~TVGSSequenceComponent() override = default;

		static std::string StaticGetTypeNameID()
		{
			return T{}.GetTypeNameID();
		}

		Ref<IVGSSequenceComponent> Clone() override
		{
			auto instance = MakeRef<T>();
			*instance = * dynamic_cast<T*>(this);
			return instance;
		}
	};

	// Manager for sequence components, responsible for registering component types and creating instances by type name ID
	// 序列组件的管理器，负责注册组件类型和通过类型名称 ID 创建实例
	struct VG_GSS_API IVGSSequenceComponentManager
	{
		IVGSSequenceComponentManager();
		~IVGSSequenceComponentManager() = default;

		static IVGSSequenceComponentManager& Get();

		template<class T>
		void EmplaceComponentType()
		{
			T instance;
			RegisteredComponents.emplace(instance.GetTypeNameID(), MakeRef<T>());
		}

		Ref<IVGSSequenceComponent> CreateSequenceEntryByTypeNameID(const std::string& typeNameID);

		/// Fills `outSorted` with registered type name IDs (sorted) for editor bootstrap / tooling.
		void EnumerateRegisteredTypeNameIDs(std::vector<std::string>& outSorted) const;

		std::unordered_map<std::string, Ref<IVGSSequenceComponent>> RegisteredComponents;
	};

	VG_GSS_API Ref<IVGSSequenceComponent> CreateSequenceEntryByTypeNameID(const std::string& typeNameID);
}
