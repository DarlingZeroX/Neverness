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

#include <memory>
#include <string>
#include <unordered_map>

#include <HCore/Include/File/nlohmann/json.hpp>

#include "../../GSSExport.h"
#include "DataContainer.h"

namespace VisionGal
{
	// -------------------------------------------------------------------------
	// VGSSequenceDataContainer 的 JSON 序列化（nlohmann::json）
	//
	// 设计要点（中文说明）：
	// 1. 容器层只负责「条目顺序、类型判别、公共字段（如 SequenceIndex）」；
	//    各具体组件的成员字段由「组件绑定」分别读写，便于扩展新组件类型。
	// 2. 单个序列条目的 JSON 形状约定为：
	//    {
	//      "type": "<与 IVGSSequenceComponent::GetTypeNameID() 一致>",
	//      "sequenceIndex": <unsigned>,
	//      "data": { ... 仅组件自有字段，由对应绑定写入 ... }
	//    }
	// 3. 扩展新组件的步骤（无需修改容器序列化代码）：
	//    a) 在你的 .cpp 中，于命名空间 VisionGal 内提供：
	//         void component_to_json(const YourComponent&, nlohmann::json& out);
	//         void component_from_json(const nlohmann::json& in, YourComponent& out);
	//       （依赖 ADL：与组件类型同命名空间即可被模板绑定找到）
	//    b) 使用 TVGSSequenceComponentJsonBinding<YourComponent> 向注册表注册，
	//       并在 IVGSSequenceComponentManager 中注册创建工厂（若尚未注册）。
	// -------------------------------------------------------------------------

	/// JSON 根对象中的格式版本字段名（便于日后迁移旧存档）
	inline constexpr const char* kVGSSequenceJsonFormatVersionKey = "formatVersion";

	/// 当前写入的格式版本号；读入时可根据版本做兼容分支
	inline constexpr int kVGSSequenceJsonFormatVersion = 1;

	inline constexpr const char* kVGSSequenceJsonSequenceKey = "sequence";
	inline constexpr const char* kVGSSequenceJsonEntryTypeKey = "type";
	inline constexpr const char* kVGSSequenceJsonEntrySequenceIndexKey = "sequenceIndex";
	inline constexpr const char* kVGSSequenceJsonEntryDataKey = "data";

	/**
	 * 单个组件类型对应的 JSON 读写接口（多态擦除）。
	 * 一般业务侧不直接使用，而是通过 TVGSSequenceComponentJsonBinding<T> + 注册表扩展。
	 */
	struct VG_GSS_API IVGSSequenceComponentJsonBinding
	{
		virtual ~IVGSSequenceComponentJsonBinding() = default;

		/// 将具体组件的自有字段写入 `out`（不含 type / sequenceIndex，由容器层写入外层）
		virtual void SerializeData(const IVGSSequenceComponent* component, nlohmann::json& out) const = 0;

		/// 从 `in` 读取自有字段写入已创建的组件实例（不含 sequenceIndex）
		virtual bool DeserializeData(const nlohmann::json& in, IVGSSequenceComponent* component) const = 0;
	};

	using VGSSequenceComponentJsonBindingPtr = std::shared_ptr<IVGSSequenceComponentJsonBinding>;

	/**
	 * 组件类型 -> JSON 绑定的注册表（单例）。
	 * 反序列化时：先用 type 字符串 CreateSequenceEntryByTypeNameID，再用绑定填充 data。
	 */
	struct VG_GSS_API VGSSequenceComponentJsonRegistry
	{
		static VGSSequenceComponentJsonRegistry& Get();

		/// typeNameID 必须与 IVGSSequenceComponent::GetTypeNameID() 返回值一致
		void Register(std::string typeNameID, VGSSequenceComponentJsonBindingPtr binding);

		/// 若未注册，SerializeData 写入空对象 {}，DeserializeData 返回 false
		VGSSequenceComponentJsonBindingPtr Find(const std::string& typeNameID) const;

	private:
		VGSSequenceComponentJsonRegistry() = default;

		std::unordered_map<std::string, VGSSequenceComponentJsonBindingPtr> m_BindingsByType;
	};

	/**
	 * 通用模板绑定：通过 ADL 查找 component_to_json / component_from_json。
	 *
	 * 要求在命名空间 VisionGal 中为具体类型 T 提供：
	 *   void component_to_json(const T&, nlohmann::json& out);
	 *   void component_from_json(const nlohmann::json& in, T& out);
	 */
	template<class T>
	struct TVGSSequenceComponentJsonBinding final : IVGSSequenceComponentJsonBinding
	{
		void SerializeData(const IVGSSequenceComponent* component, nlohmann::json& out) const override
		{
			const auto* concrete = dynamic_cast<const T*>(component);
			if (!concrete)
				return;
			component_to_json(*concrete, out);
		}

		bool DeserializeData(const nlohmann::json& in, IVGSSequenceComponent* component) const override
		{
			auto* concrete = dynamic_cast<T*>(component);
			if (!concrete)
				return false;
			component_from_json(in, *concrete);
			return true;
		}
	};

	/// 将容器序列化为 JSON 对象（含 formatVersion 与 sequence 数组）
	VG_GSS_API nlohmann::json SerializeVGSSequenceDataContainerToJson(
		const VGSSequenceDataContainer& container);

	/**
	 * 从 JSON 对象恢复容器。
	 * @return 全部条目均成功解析时为 true；任一条失败（未知类型、绑定缺失、字段异常）则为 false，
	 *         且 out 会保持调用前的内容不变（实现上先解析到临时容器再交换）。
	 */
	VG_GSS_API bool DeserializeVGSSequenceDataContainerFromJson(
		const nlohmann::json& root,
		VGSSequenceDataContainer& out);

	/// 便捷：序列化为字符串（indent < 0 表示紧凑输出）
	VG_GSS_API std::string SerializeVGSSequenceDataContainerToString(
		const VGSSequenceDataContainer& container,
		int indent = 0);

	/// 便捷：从字符串解析；解析失败或内容校验失败返回 false
	VG_GSS_API bool DeserializeVGSSequenceDataContainerFromString(
		const std::string& text,
		VGSSequenceDataContainer& out);
}
