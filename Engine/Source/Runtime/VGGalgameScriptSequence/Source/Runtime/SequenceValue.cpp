/*
 * SequenceValue — JSON 与 variant 互转实现
 */

#include "Runtime/SequenceValue.h"

#include <optional>

namespace VisionGal::GalGame
{
	namespace
	{
		const char* TypeToString(const SequenceValueType t) noexcept
		{
			switch (t)
			{
			case SequenceValueType::Int: return "int";
			case SequenceValueType::Float: return "float";
			case SequenceValueType::Bool: return "bool";
			case SequenceValueType::String: return "string";
			}
			return "bool";
		}

		std::optional<SequenceValueType> TypeFromString(const std::string& s) noexcept
		{
			if (s == "int") return SequenceValueType::Int;
			if (s == "float") return SequenceValueType::Float;
			if (s == "bool") return SequenceValueType::Bool;
			if (s == "string") return SequenceValueType::String;
			return std::nullopt;
		}
	}

	SequenceValue SequenceValue::MakeInt(const int v)
	{
		SequenceValue o;
		o.type = SequenceValueType::Int;
		o.data = v;
		return o;
	}

	SequenceValue SequenceValue::MakeFloat(const float v)
	{
		SequenceValue o;
		o.type = SequenceValueType::Float;
		o.data = v;
		return o;
	}

	SequenceValue SequenceValue::MakeBool(const bool v)
	{
		SequenceValue o;
		o.type = SequenceValueType::Bool;
		o.data = v;
		return o;
	}

	SequenceValue SequenceValue::MakeString(std::string v)
	{
		SequenceValue o;
		o.type = SequenceValueType::String;
		o.data = std::move(v);
		return o;
	}

	nlohmann::json SequenceValue::ToJson() const
	{
		nlohmann::json j;
		j["type"] = TypeToString(type);
		switch (type)
		{
		case SequenceValueType::Int:
			j["value"] = std::get<int>(data);
			break;
		case SequenceValueType::Float:
			j["value"] = std::get<float>(data);
			break;
		case SequenceValueType::Bool:
			j["value"] = std::get<bool>(data);
			break;
		case SequenceValueType::String:
			j["value"] = std::get<std::string>(data);
			break;
		}
		return j;
	}

	bool SequenceValue::TryFromJson(const nlohmann::json& j, SequenceValue& out)
	{
		if (!j.is_object() || !j.contains("type") || !j["type"].is_string())
			return false;
		const std::optional<SequenceValueType> t = TypeFromString(j["type"].get<std::string>());
		if (!t.has_value())
			return false;
		if (!j.contains("value"))
			return false;

		SequenceValue tmp;
		tmp.type = *t;
		try
		{
			switch (tmp.type)
			{
			case SequenceValueType::Int:
				tmp.data = j["value"].get<int>();
				break;
			case SequenceValueType::Float:
				tmp.data = j["value"].get<float>();
				break;
			case SequenceValueType::Bool:
				tmp.data = j["value"].get<bool>();
				break;
			case SequenceValueType::String:
				tmp.data = j["value"].get<std::string>();
				break;
			}
		}
		catch (const nlohmann::json::type_error&)
		{
			return false;
		}
		out = std::move(tmp);
		return true;
	}
}
