#pragma once
#include <string>
#include <sstream>

namespace Horizon
{
	class HSequenceWStringGenerator
	{
	public:
		HSequenceWStringGenerator(const std::wstring& prefix)
			: m_Prefix(prefix)
			, m_Index(0)
		{
		}

		std::wstring GetNext()
		{
			std::wstringstream ss;
			ss << m_Prefix << m_Index;
			m_Index++;
			return ss.str();
		}

	private:
		std::wstring m_Prefix;
		int m_Index;
	};

	class HSequenceStringGenerator
	{
	public:
		HSequenceStringGenerator(const std::string& prefix)
			: m_Prefix(prefix)
			, m_Index(0)
		{
		}

		std::string GetNext()
		{
			std::stringstream ss;
			ss << m_Prefix << m_Index;
			m_Index++;
			return ss.str();
		}

	private:
		std::string m_Prefix;
		int m_Index;
	};
}
