#include "VGLuaCore/LuaLocalizator.h"
#include <unordered_set>
#include <HCore/Include/Core/HLocalization.h>

//const char* VGLuaCoreLocalize(const char* msg)
//{
//	static std::unordered_set <std::string> s_StringCache;
//
//	auto* localizator = Horizon::HLocalizationManager::GetInstance();
//	auto language = localizator->GetCurrentLanguage();
//	auto text = localizator->Translate(msg);
//	s_StringCache.insert(text);
//
//	return msg; // 找不到映射则返回原消息
//}

const char* VGLuaCoreLocalize(const char* msg)
{
	if (!msg || !*msg) {
		return msg; // 处理空指针或空字符串
	}

	auto* localizator = Horizon::HLocalizationManager::GetInstance();
	if (!localizator) {
		return msg; // 本地化管理器不存在
	}

	auto language = localizator->GetCurrentLanguage();
	auto text = localizator->Translate(msg);

	// 如果翻译失败或返回空，返回原消息
	if (text.empty() || text == msg) {
		return msg;
	}

	// 缓存翻译结果并返回
	static std::unordered_set<std::string> s_StringCache;
	auto it = s_StringCache.find(text);
	if (it != s_StringCache.end()) {
		return it->c_str(); // 返回已缓存的字符串
	}

	// 插入新翻译并返回
	auto result = s_StringCache.insert(text);
	return result.first->c_str();
}
