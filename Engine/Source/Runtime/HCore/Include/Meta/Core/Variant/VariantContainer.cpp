#include "pch.h"

#include "VariantContainer.h"

#include "../../Type.h"

#include <algorithm>

BEGIN_META_NAMESPACE
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// void
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#pragma region void

VariantContainer<void>::VariantContainer(void) { }

Type VariantContainer<void>::GetType(void) const
{
	return TypeOf(void);
}

///////////////////////////////////////////////////////////////////////

void* VariantContainer<void>::GetPtr(void) const
{
	return nullptr;
}

///////////////////////////////////////////////////////////////////////

int VariantContainer<void>::ToInt(void) const
{
	return int();
}

///////////////////////////////////////////////////////////////////////

bool VariantContainer<void>::ToBool(void) const
{
	return bool();
}

///////////////////////////////////////////////////////////////////////

float VariantContainer<void>::ToFloat(void) const
{
	return float();
}

///////////////////////////////////////////////////////////////////////

double VariantContainer<void>::ToDouble(void) const
{
	return double();
}

///////////////////////////////////////////////////////////////////////

std::string VariantContainer<void>::ToString(void) const
{
	return std::string();
}

std::int8_t VariantContainer<void>::ToInt8() const
{
	return 0;
}

std::int16_t VariantContainer<void>::ToInt16() const
{
	return 0;
}

std::int32_t VariantContainer<void>::ToInt32() const
{
	return 0;
}

std::int64_t VariantContainer<void>::ToInt64() const
{
	return 0;
}

std::uint8_t VariantContainer<void>::ToUint8() const
{
	return 0;
}

std::uint16_t VariantContainer<void>::ToUint16() const
{
	return 0;
}

std::uint32_t VariantContainer<void>::ToUint32() const
{
	return 0;
}

std::uint64_t VariantContainer<void>::ToUint64() const
{
	return 0;
}

long VariantContainer<void>::ToLong() const
{
	return 0;
}

///////////////////////////////////////////////////////////////////////

VariantBase* VariantContainer<void>::Clone(void) const
{
	return new VariantContainer<void>();
}

#pragma endregion

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// int
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#pragma region int

VariantContainer<int>::VariantContainer(const int& value)
	: m_value(value) { }

///////////////////////////////////////////////////////////////////////

VariantContainer<int>::VariantContainer(const int&& value)
	: m_value(std::move(value)) { }

///////////////////////////////////////////////////////////////////////

Type VariantContainer<int>::GetType(void) const
{
	return TypeOf(int);
}

///////////////////////////////////////////////////////////////////////

void* VariantContainer<int>::GetPtr(void) const
{
	return const_cast<void*>(
		reinterpret_cast<const void*>(
			std::addressof(m_value)
			)
		);
}

///////////////////////////////////////////////////////////////////////

int VariantContainer<int>::ToInt(void) const
{
	return m_value;
}

///////////////////////////////////////////////////////////////////////

bool VariantContainer<int>::ToBool(void) const
{
	return m_value == 0 ? false : true;
}

///////////////////////////////////////////////////////////////////////

float VariantContainer<int>::ToFloat(void) const
{
	return static_cast<float>(m_value);
}

///////////////////////////////////////////////////////////////////////

double VariantContainer<int>::ToDouble(void) const
{
	return static_cast<double>(m_value);
}

///////////////////////////////////////////////////////////////////////

std::string VariantContainer<int>::ToString(void) const
{
	return std::to_string(m_value);
}

///////////////////////////////////////////////////////////////////////

VariantBase* VariantContainer<int>::Clone(void) const
{
	return new VariantContainer<int>(m_value);
}


std::int8_t VariantContainer<int>::ToInt8() const
{
	return m_value;
}

std::int16_t VariantContainer<int>::ToInt16() const
{
	return m_value;
}

std::int32_t VariantContainer<int>::ToInt32() const
{
	return m_value;
}

std::int64_t VariantContainer<int>::ToInt64() const
{
	return m_value;
}

std::uint8_t VariantContainer<int>::ToUint8() const
{
	return m_value;
}

std::uint16_t VariantContainer<int>::ToUint16() const
{
	return m_value;
}

std::uint32_t VariantContainer<int>::ToUint32() const
{
	return m_value;
}

std::uint64_t VariantContainer<int>::ToUint64() const
{
	return m_value;
}

long VariantContainer<int>::ToLong() const
{
	return m_value;
}

#pragma endregion

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// unsigned int
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#pragma region unsigned unsigned

VariantContainer<unsigned>::VariantContainer(const unsigned& value)
	: m_value(value) { }

///////////////////////////////////////////////////////////////////////

VariantContainer<unsigned>::VariantContainer(const unsigned&& value)
	: m_value(std::move(value)) { }

///////////////////////////////////////////////////////////////////////

Type VariantContainer<unsigned>::GetType(void) const
{
	return TypeOf(unsigned);
}

///////////////////////////////////////////////////////////////////////

void* VariantContainer<unsigned>::GetPtr(void) const
{
	return const_cast<void*>(
		reinterpret_cast<const void*>(
			std::addressof(m_value)
			)
		);
}

///////////////////////////////////////////////////////////////////////

int VariantContainer<unsigned>::ToInt(void) const
{
	return static_cast<int>(m_value);
}

///////////////////////////////////////////////////////////////////////

bool VariantContainer<unsigned>::ToBool(void) const
{
	return m_value == 0 ? false : true;
}

///////////////////////////////////////////////////////////////////////

float VariantContainer<unsigned>::ToFloat(void) const
{
	return static_cast<float>(m_value);
}

///////////////////////////////////////////////////////////////////////

double VariantContainer<unsigned>::ToDouble(void) const
{
	return static_cast<double>(m_value);
}

///////////////////////////////////////////////////////////////////////

std::string VariantContainer<unsigned>::ToString(void) const
{
	return std::to_string(m_value);
}

///////////////////////////////////////////////////////////////////////

VariantBase* VariantContainer<unsigned>::Clone(void) const
{
	return new VariantContainer<unsigned>(m_value);
}

std::int8_t VariantContainer<unsigned>::ToInt8() const
{
	return m_value;
}

std::int16_t VariantContainer<unsigned>::ToInt16() const
{
	return m_value;
}

std::int32_t VariantContainer<unsigned>::ToInt32() const
{
	return m_value;
}

std::int64_t VariantContainer<unsigned>::ToInt64() const
{
	return m_value;
}

std::uint8_t VariantContainer<unsigned>::ToUint8() const
{
	return m_value;
}

std::uint16_t VariantContainer<unsigned>::ToUint16() const
{
	return m_value;
}

std::uint32_t VariantContainer<unsigned>::ToUint32() const
{
	return m_value;
}

std::uint64_t VariantContainer<unsigned>::ToUint64() const
{
	return m_value;
}

long VariantContainer<unsigned>::ToLong() const
{
	return m_value;
}

#pragma endregion

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// bool
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#pragma region bool

VariantContainer<bool>::VariantContainer(const bool& value)
	: m_value(value) { }

///////////////////////////////////////////////////////////////////////

VariantContainer<bool>::VariantContainer(const bool&& value)
	: m_value(std::move(value)) { }

///////////////////////////////////////////////////////////////////////

Type VariantContainer<bool>::GetType(void) const
{
	return TypeOf(bool);
}

///////////////////////////////////////////////////////////////////////

void* VariantContainer<bool>::GetPtr(void) const
{
	return const_cast<void*>(
		reinterpret_cast<const void*>(
			std::addressof(m_value)
			)
		);
}

///////////////////////////////////////////////////////////////////////

int VariantContainer<bool>::ToInt(void) const
{
	return m_value ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////

bool VariantContainer<bool>::ToBool(void) const
{
	return m_value;
}

///////////////////////////////////////////////////////////////////////

float VariantContainer<bool>::ToFloat(void) const
{
	return m_value ? 1.0f : 0.0f;
}

///////////////////////////////////////////////////////////////////////

double VariantContainer<bool>::ToDouble(void) const
{
	return m_value ? 1.0 : 0.0;
}

///////////////////////////////////////////////////////////////////////

std::string VariantContainer<bool>::ToString(void) const
{
	return m_value ? "true" : "false";
}

///////////////////////////////////////////////////////////////////////

VariantBase* VariantContainer<bool>::Clone(void) const
{
	return new VariantContainer<bool>(m_value);
}

std::int8_t VariantContainer<bool>::ToInt8() const
{
	return m_value;
}

std::int16_t VariantContainer<bool>::ToInt16() const
{
	return m_value;
}

std::int32_t VariantContainer<bool>::ToInt32() const
{
	return m_value;
}

std::int64_t VariantContainer<bool>::ToInt64() const
{
	return m_value;
}

std::uint8_t VariantContainer<bool>::ToUint8() const
{
	return m_value;
}

std::uint16_t VariantContainer<bool>::ToUint16() const
{
	return m_value;
}

std::uint32_t VariantContainer<bool>::ToUint32() const
{
	return m_value;
}

std::uint64_t VariantContainer<bool>::ToUint64() const
{
	return m_value;
}

long VariantContainer<bool>::ToLong() const
{
	return m_value;
}

#pragma endregion

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// float
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#pragma region float

VariantContainer<float>::VariantContainer(const float& value)
	: m_value(value) { }

///////////////////////////////////////////////////////////////////////

VariantContainer<float>::VariantContainer(const float&& value)
	: m_value(std::move(value)) { }

///////////////////////////////////////////////////////////////////////

Type VariantContainer<float>::GetType(void) const
{
	return TypeOf(float);
}

///////////////////////////////////////////////////////////////////////

void* VariantContainer<float>::GetPtr(void) const
{
	return const_cast<void*>(
		reinterpret_cast<const void*>(
			std::addressof(m_value)
			)
		);
}

///////////////////////////////////////////////////////////////////////

int VariantContainer<float>::ToInt(void) const
{
	return static_cast<int>(m_value);
}

///////////////////////////////////////////////////////////////////////

bool VariantContainer<float>::ToBool(void) const
{
	return m_value == 0.0f ? false : true;
}

///////////////////////////////////////////////////////////////////////

float VariantContainer<float>::ToFloat(void) const
{
	return m_value;
}

///////////////////////////////////////////////////////////////////////

double VariantContainer<float>::ToDouble(void) const
{
	return static_cast<double>(m_value);
}

///////////////////////////////////////////////////////////////////////

std::string VariantContainer<float>::ToString(void) const
{
	return std::to_string(m_value);
}

///////////////////////////////////////////////////////////////////////

VariantBase* VariantContainer<float>::Clone(void) const
{
	return new VariantContainer<float>(m_value);
}

std::int8_t VariantContainer<float>::ToInt8() const
{
	return m_value;
}

std::int16_t VariantContainer<float>::ToInt16() const
{
	return m_value;
}

std::int32_t VariantContainer<float>::ToInt32() const
{
	return m_value;
}

std::int64_t VariantContainer<float>::ToInt64() const
{
	return m_value;
}

std::uint8_t VariantContainer<float>::ToUint8() const
{
	return m_value;
}

std::uint16_t VariantContainer<float>::ToUint16() const
{
	return m_value;
}

std::uint32_t VariantContainer<float>::ToUint32() const
{
	return m_value;
}

std::uint64_t VariantContainer<float>::ToUint64() const
{
	return m_value;
}

long VariantContainer<float>::ToLong() const
{
	return m_value;
}

#pragma endregion

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// double
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#pragma region double

VariantContainer<double>::VariantContainer(const double& value)
	: m_value(value) { }

///////////////////////////////////////////////////////////////////////

VariantContainer<double>::VariantContainer(const double&& value)
	: m_value(std::move(value)) { }

///////////////////////////////////////////////////////////////////////

Type VariantContainer<double>::GetType(void) const
{
	return TypeOf(double);
}

///////////////////////////////////////////////////////////////////////

void* VariantContainer<double>::GetPtr(void) const
{
	return const_cast<void*>(
		reinterpret_cast<const void*>(
			std::addressof(m_value)
			)
		);
}

///////////////////////////////////////////////////////////////////////

int VariantContainer<double>::ToInt(void) const
{
	return static_cast<int>(m_value);
}

///////////////////////////////////////////////////////////////////////

bool VariantContainer<double>::ToBool(void) const
{
	return m_value == 0.0 ? false : true;
}

///////////////////////////////////////////////////////////////////////

float VariantContainer<double>::ToFloat(void) const
{
	return static_cast<float>(m_value);
}

///////////////////////////////////////////////////////////////////////

double VariantContainer<double>::ToDouble(void) const
{
	return m_value;
}

///////////////////////////////////////////////////////////////////////

std::string VariantContainer<double>::ToString(void) const
{
	return std::to_string(m_value);
}

///////////////////////////////////////////////////////////////////////

VariantBase* VariantContainer<double>::Clone(void) const
{
	return new VariantContainer<double>(m_value);
}

std::int8_t VariantContainer<double>::ToInt8() const
{
	return m_value;
}

std::int16_t VariantContainer<double>::ToInt16() const
{
	return m_value;
}

std::int32_t VariantContainer<double>::ToInt32() const
{
	return m_value;
}

std::int64_t VariantContainer<double>::ToInt64() const
{
	return m_value;
}

std::uint8_t VariantContainer<double>::ToUint8() const
{
	return m_value;
}

std::uint16_t VariantContainer<double>::ToUint16() const
{
	return m_value;
}

std::uint32_t VariantContainer<double>::ToUint32() const
{
	return m_value;
}

std::uint64_t VariantContainer<double>::ToUint64() const
{
	return m_value;
}

long VariantContainer<double>::ToLong() const
{
	return m_value;
}

#pragma endregion

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// string
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#pragma region string

VariantContainer<std::string>::VariantContainer(
	const std::string& value
)
	: m_value(value) { }

///////////////////////////////////////////////////////////////////////

VariantContainer<std::string>::VariantContainer(
	const std::string&& value
)
	: m_value(std::move(value)) { }

///////////////////////////////////////////////////////////////////////

Type VariantContainer<std::string>::GetType(void) const
{
	return TypeOf(std::string);
}

///////////////////////////////////////////////////////////////////////

void* VariantContainer<std::string>::GetPtr(void) const
{
	return const_cast<void*>(
		reinterpret_cast<const void*>(
			std::addressof(m_value)
			)
		);
}

///////////////////////////////////////////////////////////////////////

int VariantContainer<std::string>::ToInt(void) const
{
	return stoi(m_value);
}

///////////////////////////////////////////////////////////////////////

bool VariantContainer<std::string>::ToBool(void) const
{
	// 0 -> false
	// 1 -> true
	// "true" -> true (case insensitive)
	// "false" -> false (case insensitive)

	if (m_value == "0")
		return false;

	if (m_value == "1")
		return true;

	auto copy = m_value;

	// convert to lowercase
	transform(copy.begin(), copy.end(), copy.begin(), tolower);

	if (copy == "true")
		return true;

	return false;
}

///////////////////////////////////////////////////////////////////////

float VariantContainer<std::string>::ToFloat(void) const
{
	return stof(m_value);
}

///////////////////////////////////////////////////////////////////////

double VariantContainer<std::string>::ToDouble(void) const
{
	return stod(m_value);
}

///////////////////////////////////////////////////////////////////////

std::string VariantContainer<std::string>::ToString(void) const
{
	return m_value;
}

///////////////////////////////////////////////////////////////////////

VariantBase* VariantContainer<std::string>::Clone(void) const
{
	return new VariantContainer<std::string>(m_value);
}

std::int8_t VariantContainer<std::string>::ToInt8() const
{
	return std::stoi(m_value);
}

std::int16_t VariantContainer<std::string>::ToInt16() const
{
	return std::stoi(m_value);
}

std::int32_t VariantContainer<std::string>::ToInt32() const
{
	return std::stoi(m_value);
}

std::int64_t VariantContainer<std::string>::ToInt64() const
{
	return std::stol(m_value);
}

std::uint8_t VariantContainer<std::string>::ToUint8() const
{
	return std::stoul(m_value);
}

std::uint16_t VariantContainer<std::string>::ToUint16() const
{
	return std::stoul(m_value);
}

std::uint32_t VariantContainer<std::string>::ToUint32() const
{
	return std::stol(m_value);
}

std::uint64_t VariantContainer<std::string>::ToUint64() const
{
	return std::stoul(m_value);
}

long VariantContainer<std::string>::ToLong() const
{
	return std::stol(m_value);
}

#pragma endregion
END_META_NAMESPACE