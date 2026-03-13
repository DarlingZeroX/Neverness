// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include "../CoreModuleDefinitions.h"
#include <filesystem>
//#ifndef RAPIDJSON_SSE2
//#define RAPIDJSON_SSE2
//#endif // !RAPIDJSON_SSE2

#pragma warning(push)
#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26819)
// 26812: The enum type 'x' is unscoped. Prefer 'enum class' over 'enum' (Enum.3).

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#pragma warning(pop)

#include <iostream>
#include <fstream>
//#include "../System/HFileSystem.h"
#include <HCore/Include/Meta/Meta.h>

namespace rjson = rapidjson;

namespace Horizon
{
	struct CORE_MODULE_API HJson
	{
		HJson()
		{
			m_Doc = CreateRef<rjson::Document>();

			m_Doc->SetObject();
		}

		HJson(const HJson& other)
		{
			m_Doc = other.m_Doc;
		}

		bool LoadJson(const std::filesystem::path& path);

		bool LoadJson(std::fstream& stream);

		bool SaveJson(const std::filesystem::path& path);

		void SaveJson(std::fstream& stream);

		inline auto& doc()
		{
			return *m_Doc;
		}

		inline auto& allocator()
		{
			return m_Doc->GetAllocator();
		}

		inline auto& operator()()
		{
			return doc();
		}

		inline auto& AddArray(const std::string& keyStr)
		{
			rjson::Value key(keyStr.c_str(), allocator()); // copy string name

			return m_Doc->AddMember(key, rjson::Value().SetArray(), allocator());
		}

		inline auto& AddArray(rjson::Value& value, const std::string& keyStr)
		{
			rjson::Value key(keyStr.c_str(), allocator()); // copy string name

			return value.AddMember(key, rjson::Value().SetArray(), allocator());
		}

		//template<size_t T>
		template<size_t N>
		inline auto& AddObject(const char(&str)[N])
		{
			return m_Doc->AddMember(str, rjson::Value().SetObject(), allocator());
		}

		//template<size_t T>
		template<size_t N>
		inline auto& AddArray(const char(&str)[N])
		{
			return m_Doc->AddMember(str, rjson::Value().SetArray(), allocator());
		}

		////////////////////////////////////////
		///	Constexpr key
		////////////////////////////////////////
		template<size_t N, typename T>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], T data)
		{
			return container.AddMember(key, rjson::Value(data), allocator()); // copy string name)
		}

		template<size_t N>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], rjson::Value& data)
		{
			return container.AddMember(key, data, allocator()); // copy string name)
		}

		template<size_t N>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], const char* data)
		{
			return container.AddMember(key, rjson::Value(data, allocator()), allocator()); // copy string name)
		}

		template<size_t N>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], const std::string& data)
		{
			return container.AddMember(key, rjson::Value(data.c_str(), allocator()), allocator()); // copy string name)
		}

		template<size_t N>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], const int2& data)
		{
			rjson::Value value;
			value.SetArray();

			value.PushBack(data.x, allocator());
			value.PushBack(data.y, allocator());

			return container.AddMember(key, value, allocator()); // copy string name)
		}

		template<size_t N>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], const int3& data)
		{
			rjson::Value value;
			value.SetArray();

			value.PushBack(data.x, allocator());
			value.PushBack(data.y, allocator());
			value.PushBack(data.z, allocator());

			return container.AddMember(key, value, allocator()); // copy string name)
		}

		template<size_t N>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], const int4& data)
		{
			rjson::Value value;
			value.SetArray();

			value.PushBack(data.x, allocator());
			value.PushBack(data.y, allocator());
			value.PushBack(data.z, allocator());
			value.PushBack(data.w, allocator());

			return container.AddMember(key, value, allocator()); // copy string name)
		}

		template<size_t N>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], const float2& data)
		{
			rjson::Value value;
			value.SetArray();

			value.PushBack(data.x, allocator());
			value.PushBack(data.y, allocator());

			return container.AddMember(key, value, allocator()); // copy string name)
		}

		template<size_t N>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], const float3& data)
		{
			rjson::Value value;
			value.SetArray();

			value.PushBack(data.x, allocator());
			value.PushBack(data.y, allocator());
			value.PushBack(data.z, allocator());

			return container.AddMember(key, value, allocator()); // copy string name)
		}

		template<size_t N>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], const float4& data)
		{
			rjson::Value value;
			value.SetArray();

			value.PushBack(data.x, allocator());
			value.PushBack(data.y, allocator());
			value.PushBack(data.z, allocator());
			value.PushBack(data.w, allocator());

			return container.AddMember(key, value, allocator()); // copy string name)
		}

		template<size_t N>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], const quaternion& data)
		{
			rjson::Value value;
			value.SetArray();

			value.PushBack(data.x, allocator());
			value.PushBack(data.y, allocator());
			value.PushBack(data.z, allocator());
			value.PushBack(data.w, allocator());

			return container.AddMember(key, value, allocator()); // copy string name)
		}

		template<size_t N>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], const matrix3x3& data)
		{
			rjson::Value value;
			value.SetArray();

			value.PushBack(data[0][0], allocator());

			return container.AddMember(key, value, allocator()); // copy string name)
		}

		template<size_t N>
		inline auto& AddMember(rjson::Value& container, const char(&key)[N], const matrix4x4& data)
		{
			rjson::Value value;
			value.SetArray();

			value.PushBack(data[0][0], allocator());
			value.PushBack(data[0][1], allocator());
			value.PushBack(data[0][2], allocator());
			value.PushBack(data[0][3], allocator());
			value.PushBack(data[1][0], allocator());
			value.PushBack(data[1][1], allocator());
			value.PushBack(data[1][2], allocator());
			value.PushBack(data[1][3], allocator());
			value.PushBack(data[2][0], allocator());
			value.PushBack(data[2][1], allocator());
			value.PushBack(data[2][2], allocator());
			value.PushBack(data[2][3], allocator());
			value.PushBack(data[3][0], allocator());
			value.PushBack(data[3][1], allocator());
			value.PushBack(data[3][2], allocator());
			value.PushBack(data[3][3], allocator());


			return container.AddMember(key, value, allocator()); // copy string name)
		}

		////////////////////////////////////////
		///	GetMember
		///////////////////////////////////////
		template<typename T>
		static T GetMember(rjson::Value& container)
		{
			return container.GetObj();
		}

		template<>
		inline static int2 GetMember<int2>(rjson::Value& container)
		{
			int2 result;

			auto value = container.GetArray();

			result.x = value[0].GetInt();
			result.y = value[1].GetInt();

			return result;
		}

		template<>
		inline static int3 GetMember<int3>(rjson::Value& container)
		{
			int3 result;

			auto value = container.GetArray();

			result.x = value[0].GetInt();
			result.y = value[1].GetInt();
			result.z = value[2].GetInt();

			return result;
		}

		template<>
		inline static int4 GetMember<int4>(rjson::Value& container)
		{
			int4 result;

			auto value = container.GetArray();

			result.x = value[0].GetInt();
			result.y = value[1].GetInt();
			result.z = value[2].GetInt();
			result.w = value[3].GetInt();

			return result;
		}

		template<>
		inline static float2 GetMember<float2>(rjson::Value& container)
		{
			float2 result;

			auto value = container.GetArray();

			result.x = value[0].GetFloat();
			result.y = value[1].GetFloat();

			return result;
		}

		template<>
		inline static float3 GetMember<float3>(rjson::Value& container)
		{
			float3 result;

			auto value = container.GetArray();

			result.x = value[0].GetFloat();
			result.y = value[1].GetFloat();
			result.z = value[2].GetFloat();

			return result;
		}

		template<>
		inline static float4 GetMember<float4>(rjson::Value& container)
		{
			float4 result;

			auto value = container.GetArray();

			result.x = value[0].GetFloat();
			result.y = value[1].GetFloat();
			result.z = value[2].GetFloat();
			result.w = value[3].GetFloat();

			return result;
		}

		template<>
		inline static quaternion GetMember<quaternion>(rjson::Value& container)
		{
			quaternion result;

			auto value = container.GetArray();

			result.x = value[0].GetFloat();
			result.y = value[1].GetFloat();
			result.z = value[2].GetFloat();
			result.w = value[3].GetFloat();

			return result;
		}

		template<typename T>
		static T GetMember(rjson::Value& container, const std::string& key)
		{
			return container[key.c_str()].GetObj();
		}

		template<>
		inline static int2 GetMember<int2>(rjson::Value& container, const std::string& key)
		{
			return GetMember<int2>(container[key.c_str()]);
		}

		template<>
		inline static int3 GetMember<int3>(rjson::Value& container, const std::string& key)
		{
			return GetMember<int3>(container[key.c_str()]);
		}

		template<>
		inline static int4 GetMember<int4>(rjson::Value& container, const std::string& key)
		{
			return GetMember<int4>(container[key.c_str()]);
		}

		template<>
		inline static float2 GetMember<float2>(rjson::Value& container, const std::string& key)
		{
			return GetMember<float2>(container[key.c_str()]);
		}

		template<>
		inline static float3 GetMember<float3>(rjson::Value& container, const std::string& key)
		{
			return GetMember<float3>(container[key.c_str()]);
		}

		template<>
		inline static float4 GetMember<float4>(rjson::Value& container, const std::string& key)
		{
			return GetMember<float4>(container[key.c_str()]);
		}

		template<>
		inline static quaternion GetMember<quaternion>(rjson::Value& container, const std::string& key)
		{
			return GetMember<quaternion>(container[key.c_str()]);
		}

		////////////////////
		///	AddMember Value key
		///////////////////
		template<typename T>
		inline auto& AddMember(rjson::Value& container, rjson::Value& key, T data)
		{
			return container.AddMember(key, data, allocator()); // copy string name)
		}

		template<typename T>
		inline auto& AddMemberRef(rjson::Value& container, rjson::Value& key, T& data)
		{
			return container.AddMember(key, data, allocator()); // copy string name)
		}

		auto& AddMember(rjson::Value& container, rjson::Value& key, const std::string& data)
		{
			return container.AddMember(key, rjson::Value(data.c_str(), allocator()), allocator()); // copy string name)
		}

		auto& AddMember(rjson::Value& container, rjson::Value& key, const char* data)
		{
			return container.AddMember(key, rjson::Value(data, allocator()), allocator()); // copy string name)
		}

		auto& AddMember(rjson::Value& container, rjson::Value& keyStr, const float2& data)
		{
			rjson::Value value;
			value.SetArray();

			value.PushBack(data.x, allocator());
			value.PushBack(data.y, allocator());

			return container.AddMember(keyStr, value, allocator()); // copy string name)
		}

		auto& AddMember(rjson::Value& container, rjson::Value& keyStr, const float3& data)
		{
			rjson::Value value;
			value.SetArray();

			value.PushBack(data.x, allocator());
			value.PushBack(data.y, allocator());
			value.PushBack(data.z, allocator());

			return container.AddMember(keyStr, value, allocator()); // copy string name)
		}

		auto& AddMember(rjson::Value& container, rjson::Value& keyStr, const float4& data)
		{
			rjson::Value value;
			value.SetArray();

			value.PushBack(data.x, allocator());
			value.PushBack(data.y, allocator());
			value.PushBack(data.z, allocator());
			value.PushBack(data.w, allocator());

			return container.AddMember(keyStr, value, allocator()); // copy string name)
		}

		////////////////////
		///	String key
		///////////////////
		template<typename T>
		inline auto& AddMember(rjson::Value& container, const std::string& keyStr, T data)
		{
			rjson::Value key(keyStr.c_str(), allocator()); // copy string name
			return AddMember(container, key, data); // copy string name)
		}

		template<typename T>
		inline auto& AddMemberRef(rjson::Value& container, const std::string& keyStr, T& data)
		{
			rjson::Value key(keyStr.c_str(), allocator()); // copy string name
			return AddMemberRef(container, key, data); // copy string name)
		}

		auto& AddMember(rjson::Value& container, const std::string& keyStr, const std::string& data)
		{
			rjson::Value key(keyStr.c_str(), allocator()); // copy string name
			return AddMember(container, key, data); // copy string name)
		}

		auto& AddMember(rjson::Value& container, const std::string& keyStr, const char* data)
		{
			rjson::Value key(keyStr.c_str(), allocator()); // copy string name
			return AddMember(container, key, data); // copy string name)
		}

		auto& AddMember(rjson::Value& container, const std::string& keyStr, const float2& data)
		{
			rjson::Value key(keyStr.c_str(), allocator()); // copy string name
			return AddMember(container, key, data); // copy string name)
		}

		auto& AddMember(rjson::Value& container, const std::string& keyStr, const float3& data)
		{
			rjson::Value key(keyStr.c_str(), allocator()); // copy string name
			return AddMember(container, key, data); // copy string name)
		}

		auto& AddMember(rjson::Value& container, const std::string& keyStr, const float4& data)
		{
			rjson::Value key(keyStr.c_str(), allocator()); // copy string name
			return AddMember(container, key, data); // copy string name)
		}

		////////////////////
		///	Meta
		///////////////////

		void AddMemberFieldClassVariant(rjson::Value& container, const Meta::FieldInfo& fieldInfo, const  Meta::Variant& data);

		void AddMemberFieldVariant(rjson::Value& container, const Meta::FieldInfo& fieldInfo, const Meta::Variant& data);

		void GetMemberField(rjson::Value& fieldContainer, const Meta::FieldInfo& fieldInfo, const Meta::Variant& instData);

		void GetMemberClass(rjson::Value& container, const Meta::Type& classType, const Meta::Variant& data);

		Ref<rjson::Document> m_Doc;

	};
}
