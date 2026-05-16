// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

namespace std
{
	template <>
	struct hash<const type_info*> {
		_NODISCARD size_t operator()(const type_info* typeinfo) const noexcept {
			return typeinfo->hash_code();
		}
	};

	template <>
	struct equal_to<const type_info*> {
		_NODISCARD bool operator()(const type_info* _Left, const type_info* _Right) const {
			return (*_Left) == (*_Right);
		}
	};
}

namespace Horizon
{
	template<typename T>
	using unordered_type_map = std::unordered_map<const type_info*, T, std::hash<const type_info*>, std::equal_to<const type_info*>>;
}