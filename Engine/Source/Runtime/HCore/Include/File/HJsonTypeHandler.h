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
#include "HJson.h"
#include "../Core/HTypeInfo.h"
#include "../Meta/Meta.h"

namespace Horizon
{
	struct HJsonTypeHandler
	{
		virtual void Serialize(HJson& json, rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data) = 0;

		virtual void Deserialize(rjson::Value& container, const Meta::FieldInfo& fieldInfo, Meta::Variant& data) {};
		//virtual void Deserialize(HJson& json) = 0;
	};

	struct HJsonTypeHandlerString : public HJsonTypeHandler
	{
		void Serialize(HJson& json, rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data) override
		{
			json.AddMember(container, fieldInfo.GetName(), data.ToString());
		}

		void Deserialize(rjson::Value& container, const Meta::FieldInfo& fieldInfo, Meta::Variant& data) override
		{
			fieldInfo.SetVariantValue(data, container.GetString());
		}
	};

	struct HJsonTypeHandlerUInt32 : public HJsonTypeHandler
	{
		void Serialize(HJson& json, rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data) override
		{
			json.AddMember(container, fieldInfo.GetName(), data.ToUint32());
		}

		void Deserialize(rjson::Value& container, const Meta::FieldInfo& fieldInfo, Meta::Variant& data) override
		{
			fieldInfo.SetVariantValue(data, container.GetUint());
		}
	};

	struct HJsonTypeHandlerInt32 : public HJsonTypeHandler
	{
		void Serialize(HJson& json, rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data) override
		{
			json.AddMember(container, fieldInfo.GetName(), data.ToInt32());
		}

		void Deserialize(rjson::Value& container, const Meta::FieldInfo& fieldInfo, Meta::Variant& data) override
		{
			fieldInfo.SetVariantValue(data, container.GetInt());
		}
	};

	struct HJsonTypeHandlerInt64 : public HJsonTypeHandler
	{
		void Serialize(HJson& json, rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data) override
		{
			json.AddMember(container, fieldInfo.GetName(), data.ToInt64());
		}

		void Deserialize(rjson::Value& container, const Meta::FieldInfo& fieldInfo, Meta::Variant& data) override
		{
			fieldInfo.SetVariantValue(data, container.GetInt64());
		}
	};

	struct HJsonTypeHandlerUInt64 : public HJsonTypeHandler
	{
		void Serialize(HJson& json, rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data) override
		{
			json.AddMember(container, fieldInfo.GetName(), data.ToUint64());
		}

		void Deserialize(rjson::Value& container, const Meta::FieldInfo& fieldInfo, Meta::Variant& data) override
		{
			fieldInfo.SetVariantValue(data, container.GetUint64());
		}
	};

	struct HJsonTypeHandlerFloat : public HJsonTypeHandler
	{
		void Serialize(HJson& json, rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data) override
		{
			json.AddMember(container, fieldInfo.GetName(), data.ToFloat());
		}

		void Deserialize(rjson::Value& container, const Meta::FieldInfo& fieldInfo, Meta::Variant& data) override
		{
			fieldInfo.SetVariantValue(data, container.GetFloat());
		}
	};

	struct HJsonTypeHandlerDouble : public HJsonTypeHandler
	{
		void Serialize(HJson& json, rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data) override
		{
			json.AddMember(container, fieldInfo.GetName(), data.ToDouble());
		}

		void Deserialize(rjson::Value& container, const Meta::FieldInfo& fieldInfo, Meta::Variant& data) override
		{
			fieldInfo.SetVariantValue(data, container.GetDouble());
		}
	};

	struct HJsonTypeHandlerBool : public HJsonTypeHandler
	{
		void Serialize(HJson& json, rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data) override
		{
			json.AddMember(container, fieldInfo.GetName(), data.ToBool());
		}

		void Deserialize(rjson::Value& container, const Meta::FieldInfo& fieldInfo, Meta::Variant& data) override
		{
			fieldInfo.SetVariantValue(data, container.GetBool());
		}
	};

	struct HJsonTypeHandlerVector2 : public HJsonTypeHandler
	{
		void Serialize(HJson& json, rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data) override
		{
			json.AddMember(container, fieldInfo.GetName(), data.GetValue<float2>());
		}

		void Deserialize(rjson::Value& container, const Meta::FieldInfo& fieldInfo, Meta::Variant& data) override
		{
			fieldInfo.SetVariantValue(data, HJson::GetMember<float2>(container));
		}
	};

	struct HJsonTypeHandlerVector3 : public HJsonTypeHandler
	{
		void Serialize(HJson& json, rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data) override
		{
			json.AddMember(container, fieldInfo.GetName(), data.GetValue<float3>());
		}

		void Deserialize(rjson::Value& container, const Meta::FieldInfo& fieldInfo, Meta::Variant& data) override
		{
			fieldInfo.SetVariantValue(data, HJson::GetMember<float3>(container));
		}
	};

	struct HJsonTypeHandlerVector4 : public HJsonTypeHandler
	{
		void Serialize(HJson& json, rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data) override
		{
			json.AddMember(container, fieldInfo.GetName(), data.GetValue<float4>());
		}

		void Deserialize(rjson::Value& container, const Meta::FieldInfo& fieldInfo, Meta::Variant& data) override
		{
			fieldInfo.SetVariantValue(data, HJson::GetMember<float4>(container));
		}
	};

	class CORE_MODULE_API HJsonTypeHandlerManager
	{
	public:
		HJsonTypeHandlerManager();

		static HJsonTypeHandlerManager& Instance();

		HJsonTypeHandler* TryGetHandler(const type_info* info);
	private:
		unordered_type_map<Ref<HJsonTypeHandler>> m_TypeHandler;
	};
}
