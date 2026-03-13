#include "pch.h"

#include "VariantBase.h"

#include "../Array/ArrayWrapper.h"

BEGIN_META_NAMESPACE
bool VariantBase::IsArray(void) const
{
	return false;
}

ArrayWrapper VariantBase::GetArray(void) const
{
	// invalid wrapper
	return { };
}

Type NullVariantBase::GetType() const
{
	return Type();
}

void* NullVariantBase::GetPtr() const
{
	return nullptr;
}

std::int8_t NullVariantBase::ToInt8() const
{
	return 0;
}

std::int16_t NullVariantBase::ToInt16() const
{
	return 0;
}

std::int32_t NullVariantBase::ToInt32() const
{
	return 0;
}

std::int64_t NullVariantBase::ToInt64() const
{
	return 0;
}

std::uint8_t NullVariantBase::ToUint8() const
{
	return 0;
}

std::uint16_t NullVariantBase::ToUint16() const
{
	return 0;
}

std::uint32_t NullVariantBase::ToUint32() const
{
	return 0;
}

std::uint64_t NullVariantBase::ToUint64() const
{
	return 0;
}

int NullVariantBase::ToInt() const
{
	return 0;
}

long NullVariantBase::ToLong() const
{
	return 0;
}

bool NullVariantBase::ToBool() const
{
	return false;
}

float NullVariantBase::ToFloat() const
{
	return .0f;
}

double NullVariantBase::ToDouble() const
{
	return .0;
}

std::string NullVariantBase::ToString() const
{
	return std::string();
}

VariantBase* NullVariantBase::Clone() const
{
	return nullptr;
}

NullVariantBase* NullVariantBase::Instance()
{
	static NullVariantBase* inst = new NullVariantBase();

	return inst;
}

bool NullVariantBase::IsValid() const
{
	return false;
}

END_META_NAMESPACE