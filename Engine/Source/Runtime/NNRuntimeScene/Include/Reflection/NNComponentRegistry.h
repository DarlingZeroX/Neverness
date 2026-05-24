#pragma once

/**
 * @file NNComponentRegistry.h
 * @brief 组件类型与字段元数据注册表（Phase 4-B：FNV-1a 稳定 TypeId）。
 */

#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "NNComponentFieldType.h"
#include "NNComponentTypeId.h"
#include "../Scene/NNEntity.h"
#include "../../NNRuntimeScene/NNRuntimeSceneExport.h"

namespace NN::Runtime::Scene
{
		class NNRuntimeScene;
	/** @brief FNV-1a 64-bit 字符串哈希（编译期常量，跨平台稳定）。 */
	inline constexpr std::uint64_t fnv1a_64(
		const char* str,
		const std::uint64_t hash = 14695981039346656037ULL)
	{
		return (*str == '\0')
			? hash
			: fnv1a_64(str + 1, (hash ^ static_cast<std::uint64_t>(*str)) * 1099511628211ULL);
	}

	/** @brief 单字段反射描述（POD 偏移 + 类型）。 */
	struct NN_RUNTIME_SCENE_API NNComponentFieldDesc
	{
		const char* NameUtf8 = nullptr;
		std::uint32_t Offset = 0u;
		std::uint32_t Size = 0u;
		NNComponentFieldType FieldType = NNComponentFieldType::Float;
	};

	/**
	 * @brief 组件序列化回调（通用序列化/反序列化入口）。
	 *
	 * 用于注册表驱动的场景序列化：新组件注册时提供此回调，
	 * NNSceneSerializer 通过回调实现通用序列化，无需硬编码。
	 */
	using NNSceneComponentSerializeFn = std::function<void(
		NNRuntimeScene& scene,
		NNEntity entity,
		std::vector<std::uint8_t>& outBlob)>;

	/** @brief 组件类型描述（名称、字段列表、稳定 NameHash）。 */
	struct NN_RUNTIME_SCENE_API NNComponentTypeDesc
	{
		NNComponentTypeId TypeId = NNComponentTypeIdInvalid;  // 等于 NameHash
		std::uint64_t NameHash = 0u;                          // FNV-1a(name)
		std::type_index TypeIndex{typeid(void)};
		const char* NameUtf8 = nullptr;
		std::size_t SizeBytes = 0u;
		std::vector<NNComponentFieldDesc> Fields{};
		NNSceneComponentSerializeFn SerializeFn{};            // 通用序列化回调（可选）
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

		/** @brief 通过稳定的 FNV-1a name hash 查找描述符（O(1)）。 */
		[[nodiscard]] const NNComponentTypeDesc* FindDescByNameHash(std::uint64_t nameHash) const noexcept;

		[[nodiscard]] const NNComponentFieldDesc* GetFieldByName(
			NNComponentTypeId typeId,
			const char* fieldNameUtf8) const noexcept;

		void ForEachField(
			NNComponentTypeId typeId,
			const std::function<void(const NNComponentFieldDesc&)>& visitor) const;

		[[nodiscard]] std::size_t GetRegisteredCount() const noexcept { return m_Descriptors.size(); }

		/** @brief 遍历所有已注册的组件类型描述符。 */
		void ForEachDescriptor(const std::function<void(const NNComponentTypeDesc&)>& visitor) const
		{
			for (const auto& desc : m_Descriptors)
				visitor(desc);
		}

	private:
		NNComponentTypeId RegisterType(
			std::type_index typeIndex,
			const char* nameUtf8,
			std::size_t sizeBytes,
			std::initializer_list<NNComponentFieldDesc> fields);

		std::unordered_map<std::type_index, NNComponentTypeId> m_TypeToId{};
		std::unordered_map<std::uint64_t, std::size_t> m_NameHashToDesc{};  // nameHash → m_Descriptors 下标
		std::vector<NNComponentTypeDesc> m_Descriptors{};
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

	/** @brief 自动注册组件（静态初始化期，NameUtf8 为显式稳定名称）。 */
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
