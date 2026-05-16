// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include <string>
#include "../CoreModuleDefinitions.h"
#include "HConfig.h"
#include "HCoreTypes.h"

namespace NN::Core
{
	struct CORE_MODULE_API HUUID
	{
	public:
		uint64 UUID;

	public:
		HUUID();
		HUUID(uint64 uuid);
		HUUID(const HUUID&) = default;
		HUUID& operator=(const HUUID&) = default;
		HUUID(HUUID&&) noexcept = default;
		HUUID& operator=(HUUID&&) noexcept = default;
		~HUUID() = default;

		static HUUID NewUUID();

		void Invalid();

		bool IsValid() const {return UUID != 0;}
		operator bool() const { return IsValid(); }

		explicit operator uint64_t(void) const { return UUID; }

		std::string ToString() const { return std::to_string(UUID); }
	};

}

namespace std
{
	template<>
	struct hash<NN::Core::HUUID>
	{
		std::size_t operator()(const NN::Core::HUUID& uuid) const
		{
			return hash<uint64_t>()(static_cast<uint64_t>(uuid));
			//return hash<uint64_t>()(uuid.UUID);
		}
	};

	template <>
	struct equal_to<NN::Core::HUUID> {

		//_NODISCARD constexpr bool operator()(const NN::Core::HUUID& _Left, const NN::Core::HUUID& _Right) const {
		//	return static_cast<uint64_t>(_Left) == static_cast<uint64_t>(_Right);
		//}

		_NODISCARD bool operator()(const NN::Core::HUUID& _Left, const NN::Core::HUUID& _Right) const {
			return static_cast<uint64_t>(_Left) == static_cast<uint64_t>(_Right);
		}
	};
}
