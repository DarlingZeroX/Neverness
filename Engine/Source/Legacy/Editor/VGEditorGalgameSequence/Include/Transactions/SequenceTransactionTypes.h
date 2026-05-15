/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */
#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace VisionGal::Editor
{
	enum class SequenceMutationType : uint8_t
	{
		Unknown = 0,
		Structure,
		EntryProperty,
	};

	enum class SequenceTransactionSource : uint8_t
	{
		Command,
		Undo,
		Redo,
		ExternalNotify,
	};

	enum class SequenceTransactionFlags : uint32_t
	{
		None = 0,
		StructuralChange = 1u << 0,
		PropertyEdit = 1u << 1,
	};

	inline SequenceTransactionFlags operator|(SequenceTransactionFlags a, SequenceTransactionFlags b)
	{
		return static_cast<SequenceTransactionFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

	inline SequenceTransactionFlags& operator|=(SequenceTransactionFlags& a, SequenceTransactionFlags b)
	{
		a = a | b;
		return a;
	}

	using SequenceValue = std::variant<std::monostate, bool, std::string, std::int64_t>;

	struct SequenceMutationRecord
	{
		SequenceMutationType Type = SequenceMutationType::Unknown;
		std::int32_t EntryIndex = -1;
		std::string PropertyPath;
		SequenceValue Before{};
		SequenceValue After{};
	};

	struct SequenceTransaction
	{
		std::uint64_t TransactionId = 0;
		std::uint64_t Generation = 0;
		SequenceTransactionSource Source = SequenceTransactionSource::Command;
		SequenceTransactionFlags Flags = SequenceTransactionFlags::None;
		std::vector<SequenceMutationRecord> Mutations;
	};
}
