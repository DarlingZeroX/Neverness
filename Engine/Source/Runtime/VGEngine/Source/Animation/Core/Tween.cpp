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
/**
	Tween below.
	Partly based on RmlUI http://github.com/mikke89/RmlUi
 */
#include "Animation/Core/Tween.h"
#include <utility>
#include <RmlUi/Core/Math.h>
#include <HCore/Interface/HStringTools.h>

namespace VisionGal {

	namespace TweenFunctions {

		/**
			Tweening functions below.
			Partly based on http://libclaw.sourceforge.net/tweeners.html
		 */

		static inline float square(float t)
		{
			return t * t;
		}

		static float back(float t)
		{
			return t * t * (2.70158f * t - 1.70158f);
		}

		static float bounce(float t)
		{
			if (t > 1.f - 1.f / 2.75f)
				return 1.f - 7.5625f * square(1.f - t);
			else if (t > 1.f - 2.f / 2.75f)
				return 1.0f - (7.5625f * square(1.f - t - 1.5f / 2.75f) + 0.75f);
			else if (t > 1.f - 2.5f / 2.75f)
				return 1.0f - (7.5625f * square(1.f - t - 2.25f / 2.75f) + 0.9375f);
			return 1.0f - (7.5625f * square(1.f - t - 2.625f / 2.75f) + 0.984375f);
		}

		static float circular(float t)
		{
			return 1.f - Rml::Math::SquareRoot(1.f - t * t);
		}

		static float cubic(float t)
		{
			return t * t * t;
		}

		static float elastic(float t)
		{
			if (t == 0)
				return t;
			if (t == 1)
				return t;
			return -Rml::Math::Exp(7.24f * (t - 1.f)) * Rml::Math::Sin((t - 1.1f) * 2.f * Rml::Math::RMLUI_PI / 0.4f);
		}

		static float exponential(float t)
		{
			if (t == 0)
				return t;
			if (t == 1)
				return t;
			return Rml::Math::Exp(7.24f * (t - 1.f));
		}

		static float linear(float t)
		{
			return t;
		}

		static float quadratic(float t)
		{
			return t * t;
		}

		static float quartic(float t)
		{
			return t * t * t * t;
		}

		static float quintic(float t)
		{
			return t * t * t * t * t;
		}

		static float sine(float t)
		{
			return 1.f - Rml::Math::Cos(t * Rml::Math::RMLUI_PI * 0.5f);
		}

	} // namespace TweenFunctions

	Tween::Tween(Type type, Direction direction)
	{
		if (direction & In)
			type_in = type;
		if (direction & Out)
			type_out = type;
	}
	Tween::Tween(Type type_in, Type type_out) : type_in(type_in), type_out(type_out) {}
	Tween::Tween(CallbackFnc callback, Direction direction) : callback(callback)
	{
		if (direction & In)
			type_in = Callback;
		if (direction & Out)
			type_out = Callback;
	}
	float Tween::operator()(float t) const
	{
		if (type_in != None && type_out == None)
		{
			return in(t);
		}
		if (type_in == None && type_out != None)
		{
			return out(t);
		}
		if (type_in != None && type_out != None)
		{
			return in_out(t);
		}
		return t;
	}

	void Tween::reverse()
	{
		std::swap(type_in, type_out);
	}

	bool Tween::operator==(const Tween& other) const
	{
		return type_in == other.type_in && type_out == other.type_out && callback == other.callback;
	}

	bool Tween::operator!=(const Tween& other) const
	{
		return !(*this == other);
	}

	String Tween::to_string() const
	{
		static const std::array<String, size_t(Count)> type_str = {
			{"none", "back", "bounce", "circular", "cubic", "elastic", "exponential", "linear", "quadratic", "quartic", "quintic", "sine", "callback"} };

		if (size_t(type_in) < type_str.size() && size_t(type_out) < type_str.size())
		{
			if (type_in == None && type_out == None)
			{
				return "none";
			}
			else if (type_in == type_out)
			{
				return type_str[size_t(type_in)] + String("-in-out");
			}
			else if (type_in == None)
			{
				return type_str[size_t(type_out)] + String("-out");
			}
			else if (type_out == None)
			{
				return type_str[size_t(type_in)] + String("-in");
			}
			else if (type_in != type_out)
			{
				return type_str[size_t(type_in)] + String("-in-") + type_str[size_t(type_out)] + String("-out");
			}
		}
		return "unknown";
	}

	// Pseudocode / 计划（中文）：
	// 1. 将输入的 `tween` 字符串去两端空白并转换为小写，便于不区分大小写比较。
	// 2. 准备一个映射表，将英文和常见中文缓动名称映射到对应的 `Tween::Type`。
	//    例如："linear" / "线性" -> Linear, "bounce" / "弹跳" -> Bounce 等。
	// 3. 使用 '-' 作为分隔符把字符串拆分成若干部分（也允许其它常见分隔符被预先替换为空格）。
	// 4. 根据拆分结果进行解析，支持如下格式：
	//    a) 单一类型 -> 如 "linear"，将 in 和 out 都设为该类型（等价于 in-out）。
	//    b) 带方向后缀 -> 如 "linear-in" 或 "linear-out" 或 "linear-in-out"。
	//    c) 两个不同类型 -> 如 "back-in-bounce-out"（parts: back, in, bounce, out）
	//    d) 两个类型但无方向词 -> 如 "back-bounce"，将第一个设为 in，第二个设为 out（容错处理）。
	// 5. 在解析时尽量宽容：如果找不到方向词但只有一个类型则视为 in-out；
	//    如果无法识别任何类型则返回 false。
	// 6. 成功解析后通过 `out` 参数设置 `type_in` 和 `type_out`，并返回 true。
	// 7. 对于 `Callback` 类型，只设置类型，不尝试设置回调函数指针（字符串不能还原回函数）。
	//
	// 注意：该实现不依赖外部非标准库，仅使用 std::string / STL 容器，且兼容中文别名。

	bool Tween::ParseTweenByString(const std::string& tween, Tween& out)
	{
		out = Tween{};

		// Helper: trim and to-lower
		//auto trim = [](const std::string& s) -> std::string {
		//	size_t b = 0;
		//	while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
		//	size_t e = s.size();
		//	while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
		//	return s.substr(b, e - b);
		//};
		//auto to_lower = [](std::string s) -> std::string {
		//	for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		//	return s;
		//};

		// 预处理输入字符串，去除空白并转换为小写
		std::string s = Horizon::HStringTools::TrimCopy(Horizon::HStringTools::ToLowerCopy(tween));
		//std::string s = to_lower(trim(tween));
		if (s.empty())
			return false;

		// 将常见分隔符替换为“-”以简化拆分  Replace common separators with '-' to simplify split
		for (char& c : s)
		{
			if (c == '_' || c == '/' || c == '\\' || c == ' ' || c == '\t')
				c = '-';
		}

		// 英语和中文名称到Tween::Type的映射 Mapping of english and chinese names to Tween::Type
		static const std::unordered_map<std::string, Type> name_map = {
			{"none", Type::None}, {"无", Type::None}, {"none-in-out", Type::None},
			{"back", Type::Back}, {"回弹", Type::Back}, {"backward", Type::Back},
			{"bounce", Type::Bounce}, {"弹跳", Type::Bounce}, {"bouncy", Type::Bounce},
			{"circular", Type::Circular}, {"circle", Type::Circular}, {"圆形", Type::Circular}, {"圆", Type::Circular},
			{"cubic", Type::Cubic}, {"三次", Type::Cubic},
			{"elastic", Type::Elastic}, {"弹性", Type::Elastic},
			{"exponential", Type::Exponential}, {"指数", Type::Exponential}, {"expo", Type::Exponential},
			{"linear", Type::Linear}, {"线性", Type::Linear},
			{"quadratic", Type::Quadratic}, {"二次", Type::Quadratic}, {"quad", Type::Quadratic},
			{"quartic", Type::Quartic}, {"四次", Type::Quartic},
			{"quintic", Type::Quintic}, {"五次", Type::Quintic},
			{"sine", Type::Sine}, {"正弦", Type::Sine},
			{"callback", Type::Callback}, {"回调", Type::Callback}
		};

		// 以'-'分隔 Split by '-'
		std::vector<std::string> parts;
		{
			size_t start = 0;
			for (size_t i = 0; i <= s.size(); ++i)
			{
				if (i == s.size() || s[i] == '-')
				{
					if (i > start)
						parts.emplace_back(s.substr(start, i - start));
					start = i + 1;
				}
			}
		}

		// 删除空部分（处理连续分隔符时） Remove empty parts (in case of consecutive delimiters)
		std::vector<std::string> tokens;
		for (auto& p : parts)
			if (!p.empty())
				tokens.push_back(p);

		// Quick helpers to check tokens
		auto is_in_word = [](const std::string& t) {
			return t == "in" || t == "inout" || t == "in-out" || t == "inout";
		};
		auto is_out_word = [](const std::string& t) {
			return t == "out";
		};

		// 尝试按顺序找出所有已识别的类型标记 Try to find all recognized type tokens in order
		std::vector<std::pair<size_t, Type>> found_types; // (index in tokens, type)
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			auto it = name_map.find(tokens[i]);
			if (it != name_map.end())
			{
				found_types.emplace_back(i, it->second);
			}
		}

		// 如果完全匹配名称（例如“linear”或中文名称），则快速处理
		// If exact full string matches a name (e.g. "linear" or Chinese name), handle quickly
		auto full_it = name_map.find(s);
		if (full_it != name_map.end())
		{
			// If full string indicates None, set both None.
			out.type_in = full_it->second;
			out.type_out = full_it->second;
			return true;
		}

		// Initialize as None
		Type in_type = Type::None;
		Type out_type = Type::None;
		bool has_in = false, has_out = false;

		// Case 1: pattern like "<type>-in-<type>-out" -> tokens: type,in,type,out
		// Detect explicit two-type pattern
		if (tokens.size() >= 4)
		{
			// find "in" and "out" positions
			for (size_t i = 0; i + 3 < tokens.size(); ++i)
			{
				if (is_in_word(tokens[i + 1]) && is_out_word(tokens[i + 3]))
				{
					auto it1 = name_map.find(tokens[i]);
					auto it2 = name_map.find(tokens[i + 2]);
					if (it1 != name_map.end() && it2 != name_map.end())
					{
						in_type = it1->second;
						out_type = it2->second;
						has_in = has_out = true;
						out.type_in = in_type;
						out.type_out = out_type;
						return true;
					}
				}
			}
		}

		// Case 2: tokens contain direction words and types around them.
		for (size_t i = 0; i < tokens.size(); ++i)
		{
			const std::string& tk = tokens[i];
			if (is_in_word(tk))
			{
				// try previous token as type, else next token
				if (i > 0)
				{
					auto it = name_map.find(tokens[i - 1]);
					if (it != name_map.end())
					{
						in_type = it->second;
						has_in = true;
						continue;
					}
				}
				if (i + 1 < tokens.size())
				{
					auto it = name_map.find(tokens[i + 1]);
					if (it != name_map.end())
					{
						in_type = it->second;
						has_in = true;
						continue;
					}
				}
			}
			else if (is_out_word(tk))
			{
				// try previous token as type, else next token
				if (i > 0)
				{
					auto it = name_map.find(tokens[i - 1]);
					if (it != name_map.end())
					{
						out_type = it->second;
						has_out = true;
						continue;
					}
				}
				if (i + 1 < tokens.size())
				{
					auto it = name_map.find(tokens[i + 1]);
					if (it != name_map.end())
					{
						out_type = it->second;
						has_out = true;
						continue;
					}
				}
			}
		}

		// Case 3: no explicit direction words but we found type tokens
		if (!has_in && !has_out)
		{
			if (found_types.size() == 1)
			{
				// single type -> treat as in-out (both)
				in_type = out_type = found_types[0].second;
				has_in = has_out = true;
			}
			else if (found_types.size() >= 2)
			{
				// two or more types -> take first as in, second as out
				in_type = found_types[0].second;
				out_type = found_types[1].second;
				has_in = has_out = true;
			}
		}

		// Case 4: one of in/out found only -> set the other to None (or same if appropriate)
		// For example "linear-in" should set in=linear, out=None (the caller can treat this)
		// However, prefer to set both to same if input had "-in-out" token with only one type
		// Check "xxx-in-out" pattern (tokens might be ["xxx","in","out"] or ["xxx","in-out"])
		for (size_t i = 0; i + 1 < tokens.size(); ++i)
		{
			if ((tokens[i + 1] == "inout" || tokens[i + 1] == "in-out") && !has_in && !has_out)
			{
				auto it = name_map.find(tokens[i]);
				if (it != name_map.end())
				{
					in_type = out_type = it->second;
					has_in = has_out = true;
				}
			}
		}

		// Final assignment and success check
		if (has_in) out.type_in = in_type;
		if (has_out) out.type_out = out_type;

		// If we recognized nothing, try to interpret whole tokens as single type name (fallback)
		if (!has_in && !has_out)
		{
			// Try tokens[0] as type
			if (!tokens.empty())
			{
				auto it = name_map.find(tokens[0]);
				if (it != name_map.end())
				{
					out.type_in = out.type_out = it->second;
					return true;
				}
			}
			return false;
		}

		return true;
	}

	float Tween::tween(Type type, float t) const
	{
		using namespace TweenFunctions;

		switch (type)
		{
		case Back: return back(t);
		case Bounce: return bounce(t);
		case Circular: return circular(t);
		case Cubic: return cubic(t);
		case Elastic: return elastic(t);
		case Exponential: return exponential(t);
		case Linear: return linear(t);
		case Quadratic: return quadratic(t);
		case Quartic: return quartic(t);
		case Quintic: return quintic(t);
		case Sine: return sine(t);
		case Callback:
			if (callback)
				return (*callback)(t);
			break;
		default: break;
		}
		return t;
	}

	float Tween::in(float t) const
	{
		return tween(type_in, t);
	}

	float Tween::out(float t) const
	{
		return 1.0f - tween(type_out, 1.0f - t);
	}

	float Tween::in_out(float t) const
	{
		if (t < 0.5f)
			return tween(type_in, 2.0f * t) * 0.5f;
		else
			return 0.5f + out(2.0f * t - 1.0f) * 0.5f;
	}

} // namespace Rml
