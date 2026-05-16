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
#include "DataVariant.h"
#include <NNKernel/Include/File/nlohmann/json.hpp>

namespace VisionGal
{
	class VG_CORE_API VGDataNamespace
	{
	public:
		sol::object GetVariableLua(const std::string& name, sol::this_state lua);
		void SetVariableLua(const std::string& name, sol::object obj, sol::this_state lua);

		template<class T>
		void SetVariable(const std::string& name, const T& value)
		{
			m_Data[name] = VGDataVariant(value);
		}

		template<class T>
		T GetVariable(const std::string& name) const
		{
			if (const auto it = m_Data.find(name); it != m_Data.end())
			{
				return it->second.GetValue<T>();
			}

			return T{};
		}

		bool ExistVariable(const std::string& name) const;
		bool RemoveVariable(const std::string& name);
		void Clear();

		void Serialize(nlohmann::json& json);
		void Deserialize(nlohmann::json& json);
	private:
		// 使用自定义的Variant类型存储，而不是直接存储sol::object
		std::unordered_map<std::string, VGDataVariant> m_Data;
	};
}
