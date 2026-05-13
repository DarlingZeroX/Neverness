/*
 * GalGameScriptExecutorFactory 实现（VGGalgameCore）
 *
 * 中文：维护 type → Creator 映射；加载时委托 Creator::LoadFromAsset。
 */

#include "IStoryScript.h"

namespace VisionGal::GalGame
{
	Ref<IStoryScriptExecutor> GalGameScriptExecutorFactory::LoadAssetExecutor(const String& type, const String& path)
	{
		if (path.empty())
		{
			return nullptr;
		}

		if (m_FactoryFunctions.find(type) == m_FactoryFunctions.end())
		{
			H_LOG_WARN("尝试加载未注册类型的剧情脚本执行器: %s", type.c_str());

			return nullptr;
		}

		return m_FactoryFunctions[type]->LoadFromAsset(path);
	}

	void GalGameScriptExecutorFactory::RegisterAssetExecutor(
		const String& type,
		Ref<IStoryScriptExecutorCreator> factoryFunction
	)
	{
		m_FactoryFunctions[type] = factoryFunction;
	}


	GalGameScriptExecutorFactory& GalGameScriptExecutorFactory::Get()
	{
		static GalGameScriptExecutorFactory instance;
		return instance;
	}

	std::vector<std::string> GalGameScriptExecutorFactory::GetRegisterTypes()
	{
		std::vector<std::string> types;
		for (const auto& pair : m_FactoryFunctions)
		{
			types.push_back(pair.first);
		}
		return types;
	}

	bool GalGameScriptExecutorFactory::HasExecutor(const String& type)
	{
		return m_FactoryFunctions.find(type) != m_FactoryFunctions.end();
	}
}
