/*
 * SequenceVariableTable — 读写实现
 */

#include "Runtime/SequenceVariableTable.h"

namespace VisionGal::GalGame
{
	void SequenceVariableTable::Set(std::string key, SequenceValue value)
	{
		m_Table[std::move(key)] = std::move(value);
	}

	SequenceValue SequenceVariableTable::Get(const std::string& key) const
	{
		const auto it = m_Table.find(key);
		if (it == m_Table.end())
			return SequenceValue::MakeBool(false);
		return it->second;
	}

	bool SequenceVariableTable::TryGet(const std::string& key, SequenceValue& out) const
	{
		const auto it = m_Table.find(key);
		if (it == m_Table.end())
			return false;
		out = it->second;
		return true;
	}

	bool SequenceVariableTable::Contains(const std::string& key) const noexcept
	{
		return m_Table.find(key) != m_Table.end();
	}

	void SequenceVariableTable::Clear() noexcept
	{
		m_Table.clear();
	}
}
