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
#include <string>
#include <sol/object.hpp>
#include <unordered_map>
#include <HCore/Include/File/nlohmann/json.hpp>
#include "../../VGCoreConfig.h"

namespace VisionGal
{
	class VG_CORE_API VGDataVariant
	{
	public:
		enum class Type {
			Nil = 0,
			Boolean = 1,
			Integer = 2,
			Number = 3,
			String = 4,
			Table = 5
		};

		VGDataVariant() = default;
		VGDataVariant(int value);
		VGDataVariant(bool value);
		VGDataVariant(double value);
		VGDataVariant(const std::string& value);
		~VGDataVariant() = default;

		// 从sol::object创建Variant
		static VGDataVariant FromLuaObject(sol::object obj, sol::this_state lua);
		// 转换为当前Lua状态的object
		sol::object ToLuaObject(sol::this_state lua) const;

		void SetValue(int value);
		void SetValue(bool value);
		void SetValue(double value);
		void SetValue(const std::string& value);

		int GetValueInt() const;
		bool GetValueBool() const;
		double GetValueNumber() const;
		std::string GetValueString() const;

		template<typename T>
		T GetValue() const
		{
			static_assert(false);
			return T();
		}

		template<>
		int GetValue<int>() const
		{
			H_ASSERT_TRUE(type == Type::Integer);
			return intValue;
		}

		template<>
		bool GetValue<bool>() const
		{
			H_ASSERT_TRUE(type == Type::Boolean);
			return boolValue;
		}

		template<>
		double GetValue<double>() const
		{
			H_ASSERT_TRUE(type == Type::Number);
			return doubleValue;
		}

		template<>
		std::string GetValue<std::string>() const
		{
			H_ASSERT_TRUE(type == Type::String);
			return stringValue;
		}

		void Serialize(const std::string& name, nlohmann::json& json);
		void Deserialize(nlohmann::json& json);
	private:
		Type type = Type::Nil;
		union {
			bool boolValue;
			int intValue;
			double doubleValue;
		};
		std::string stringValue;
		std::unordered_map<std::string, VGDataVariant> tableValue;
	};
}
