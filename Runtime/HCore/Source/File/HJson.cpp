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

#include "pch.h"
#include "File/HJson.h"
#include <sstream>

#pragma warning(push)
#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26819)

#include "File/rapidjson/prettywriter.h"
#include "File/rapidjson/istreamwrapper.h"

#pragma warning(pop)

#include "File/HJsonTypeHandler.h"

namespace Horizon
{
	static HJsonTypeHandlerManager m_Handlers;

	bool HJson::LoadJson(const std::filesystem::path& path)
	{
		std::ifstream ifs(path);
		if (!ifs.is_open())
		{
			ifs.close();
			return false;
		}

		rapidjson::IStreamWrapper isw(ifs);

		m_Doc->ParseStream(isw);

		return true;
	}

	bool HJson::LoadJson(std::fstream& stream)
	{
		rapidjson::IStreamWrapper isw(stream);

		m_Doc->ParseStream(isw);

		return true;
	}

	bool HJson::SaveJson(const std::filesystem::path& path)
	{
		rjson::StringBuffer buffer;
		//rjson::Writer<rjson::StringBuffer> writer(buffer);
		rjson::PrettyWriter<rjson::StringBuffer> writer(buffer);
		m_Doc->Accept(writer);

		std::ofstream ofs(path, std::ios::out);
		if (!ofs.is_open())
		{
			ofs.close();
			return false;
		}

		ofs << buffer.GetString();
		ofs.close();

		return true;
	}

	void HJson::SaveJson(std::fstream& stream)
	{
		rjson::StringBuffer buffer;
		rjson::PrettyWriter<rjson::StringBuffer> writer(buffer);
		m_Doc->Accept(writer);

		stream << buffer.GetString();

	}

	void HJson::AddMemberFieldClassVariant(rjson::Value& container, const Meta::FieldInfo& fieldInfo,
		const Meta::Variant& data)
	{
		rjson::Value value;
		value.SetObject();

		auto type = fieldInfo.GetType();

		for (const auto& field : type.GetFields())
		{
			AddMemberFieldVariant(value, field, field.GetValueRef(data));
		}

		AddMemberRef(container, fieldInfo.GetName(), value); // copy string name)
	}

	void HJson::AddMemberFieldVariant(rjson::Value& container, const Meta::FieldInfo& fieldInfo,
		const Meta::Variant& data)
	{
		auto type = fieldInfo.GetType();

		if (auto* handler = m_Handlers.TryGetHandler(type.GetTypeInfo()); handler != nullptr)
		{
			handler->Serialize(*this, container, fieldInfo, data);
			return;
		}

		if (type.IsClass())
		{
			AddMemberFieldClassVariant(container, fieldInfo, data);
			return;
		}

		rjson::Value key(fieldInfo.GetName().c_str(), allocator()); // copy string name

		if (type.IsEnum())
		{
			container.AddMember(key, data.ToInt64(), allocator()); // copy string name)
			return;
		}

		//container.AddMember(key, 1, allocator()); // copy string name)
		return;
	}

	void HJson::GetMemberField(rjson::Value& fieldContainer, const Meta::FieldInfo& fieldInfo,
		const Meta::Variant& instData)

	{
		auto type = fieldInfo.GetType();
		auto name = fieldInfo.GetName();

		if (auto* handler = m_Handlers.TryGetHandler(type.GetTypeInfo()); handler != nullptr)
		{
			handler->Deserialize(fieldContainer, fieldInfo, const_cast<Meta::Variant&>(instData));
			return;
		}

		if (type.IsClass())
		{
			auto subInstData = fieldInfo.GetValueRef(instData);

			for (const auto& field : type.GetFields())
			{
				const std::string& fieldName = field.GetName();
				if (fieldContainer.HasMember(fieldName.data()))
				{
					auto& subFieldContainer = fieldContainer[fieldName.c_str()];
					GetMemberField(subFieldContainer, field, subInstData);
				}
			}

			return;
		}

		rjson::Value key(fieldInfo.GetName().c_str(), allocator()); // copy string name

		if (type.IsEnum() && fieldContainer.IsUint64())
		{
			fieldInfo.SetVariantValue(const_cast<Meta::Variant&>(instData), fieldContainer.GetInt64());
			//container.AddMember(key, data.ToInt64(), allocator()); // copy string name)
			return;
		}

		return;
	}

	void HJson::GetMemberClass(rjson::Value& container, const Meta::Type& classType, const Meta::Variant& data)
	{
		for (const auto& field : classType.GetFields())
		{
			const std::string& fieldName = field.GetName();

			if (container.HasMember(fieldName.data()))
			{
				auto& fieldContainer = container[fieldName.data()];

				GetMemberField(fieldContainer, field, data);
			}
		}
	}
}
