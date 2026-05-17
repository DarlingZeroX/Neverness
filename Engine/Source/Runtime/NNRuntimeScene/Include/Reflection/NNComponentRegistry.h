#pragma once

/**
 * @file NNComponentRegistry.h
 * @brief 组件类型与字段元数据注册表（Phase 3：支持序列化反射与 C# 映射预留）。
 */

#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Reflection/NNComponentFieldType.h"
#include "Reflection/NNComponentTypeId.h"
#include "NNRuntimeScene/NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
	/** @brief 单字段反射描述（POD 偏移 + 类型）。 */
	struct NN_RUNTIME_SCENE_API NNComponentFieldDesc
	{
		const char* NameUtf8 = nullptr;
		std::uint32_t Offset = 0u;
		std::uint32_t Size = 0u;
		NNComponentFieldType FieldType = NNComponentFieldType::Float;
	};

	struct NN_RUNTIME_SCENE_API NNComponentTypeDesc
	{
		NNComponentTypeId TypeId = NNComponentTypeIdInvalid;
		std::type_index TypeIndex{typeid(void)};
		const char* NameUtf8 = nullptr;
		std::size_t SizeBytes = 0u;
		std::vector<NNComponentFieldDesc> Fields{};
	};

	class NN_RUNTIME_SCENE_API NNComponentRegistryGlobal;

	class NN_RUNTIME_SCENE_API NNComponentRegistry
	{
		friend class NNComponentRegistryGlobal;

	public:
		NNComponentRegistry() = default;

		template <typename T>
		NNComponentTypeId Register(const char* nameUtf8)
		{
			return RegisterType(std::type_index(typeid(T)), nameUtf8, sizeof(T), {});
		}

		NNComponentTypeId RegisterTypeWithFields(
			std::type_index typeIndex,
			const char* nameUtf8,
			std::size_t sizeBytes,
			std::initializer_list<NNComponentFieldDesc> fields);

		NNComponentTypeId RegisterTypeWithFields(
			std::type_index typeIndex,
			const char* nameUtf8,
			std::size_t sizeBytes,
			const std::vector<NNComponentFieldDesc>& fields);

		[[nodiscard]] NNComponentTypeId FindTypeId(const std::type_index& typeIndex) const noexcept;

		[[nodiscard]] const NNComponentTypeDesc* FindDesc(NNComponentTypeId typeId) const noexcept;

		[[nodiscard]] const NNComponentTypeDesc* FindDesc(const std::type_index& typeIndex) const noexcept;

		[[nodiscard]] const NNComponentFieldDesc* GetFieldByName(
			NNComponentTypeId typeId,
			const char* fieldNameUtf8) const noexcept;

		void ForEachField(
			NNComponentTypeId typeId,
			const std::function<void(const NNComponentFieldDesc&)>& visitor) const;

		[[nodiscard]] std::size_t GetRegisteredCount() const noexcept { return m_Descriptors.size(); }

	private:
		NNComponentTypeId RegisterType(
			std::type_index typeIndex,
			const char* nameUtf8,
			std::size_t sizeBytes,
			std::initializer_list<NNComponentFieldDesc> fields);

		std::unordered_map<std::type_index, NNComponentTypeId> m_TypeToId{};
		std::vector<NNComponentTypeDesc> m_Descriptors{};
		NNComponentTypeId m_NextTypeId = 1u;
	};

	class NN_RUNTIME_SCENE_API NNComponentRegistryGlobal
	{
	public:
		static NNComponentRegistryGlobal& Instance() noexcept;

		template <typename T>
		NNComponentTypeId Register(const char* nameUtf8)
		{
			return m_Registry.Register<T>(nameUtf8);
		}

		NNComponentTypeId RegisterTypeWithFields(
			std::type_index typeIndex,
			const char* nameUtf8,
			std::size_t sizeBytes,
			std::initializer_list<NNComponentFieldDesc> fields)
		{
			return m_Registry.RegisterTypeWithFields(typeIndex, nameUtf8, sizeBytes, fields);
		}

		[[nodiscard]] const NNComponentRegistry& GetRegistry() const noexcept { return m_Registry; }

		void MergeInto(NNComponentRegistry& destination) const;

	private:
		NNComponentRegistry m_Registry{};
	};

#define NN_FIELD(Type, Member, FieldTypeEnum)                                          \
	::NN::Runtime::Scene::NNComponentFieldDesc{                                        \
		#Member,                                                                      \
		static_cast<std::uint32_t>(offsetof(Type, Member)),                           \
		static_cast<std::uint32_t>(sizeof(static_cast<Type*>(nullptr)->Member)),      \
		::NN::Runtime::Scene::NNComponentFieldType::FieldTypeEnum}

#define NN_REGISTER_COMPONENT(Type, NameUtf8, ...)                                     \
	namespace NNRuntimeSceneAutoRegister_##Type                                        \
	{                                                                                  \
		struct Registrar                                                               \
		{                                                                              \
			Registrar()                                                                \
			{                                                                          \
				(void)::NN::Runtime::Scene::NNComponentRegistryGlobal::Instance()        \
					.RegisterTypeWithFields(                                             \
						std::type_index(typeid(Type)),                                   \
						NameUtf8,                                                         \
						sizeof(Type),                                                   \
						{__VA_ARGS__});                                                  \
			}                                                                          \
		};                                                                             \
		static Registrar g_registrar;                                                  \
	}
} // namespace NN::Runtime::Scene
