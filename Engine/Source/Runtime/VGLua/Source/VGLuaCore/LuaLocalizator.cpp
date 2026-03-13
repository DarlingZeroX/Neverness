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


#include "VGLuaCore/LuaLocalizator.h"
#include <string>
#include <unordered_set>
#include <HCore/Interface/HLocalization.h>

struct VGLuaCoreLocalizeImp
{
	VGLuaCoreLocalizeImp()
	{
	}

	static VGLuaCoreLocalizeImp& Get()
	{
		static VGLuaCoreLocalizeImp imp;
		return imp;
	}

	std::string Translate(const std::string& text)
	{
		auto* localizator = Horizon::HLocalizationManager::GetInstance();
		return localizator->Translate(text);
	}

	std::unordered_set<std::string> s_StringCache;
};

const char* VGLuaCoreLocalize(const char* msg)
{
	if (!msg || !*msg) {
		return msg; // 处理空指针或空字符串
	}

	// 缓存翻译结果并返回
	auto& cache = VGLuaCoreLocalizeImp::Get().s_StringCache;
	auto found = cache.find(msg);
	if (found != cache.end())
	{
		return found->c_str(); // 返回已缓存的字符串
	}

	auto text = VGLuaCoreLocalizeImp::Get().Translate(msg);

	// 如果翻译失败或返回空，返回原消息
	if (text.empty() || text == msg) {
		return msg;
	}

	// 插入新翻译并返回
	auto result = cache.insert(text);
	return result.first->c_str();
}
