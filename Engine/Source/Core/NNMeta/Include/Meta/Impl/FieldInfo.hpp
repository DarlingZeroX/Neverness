#pragma once
#include <assert.h>

BEGIN_META_NAMESPACE

//template <typename T>
//bool FieldInfo::SetValue(T& instance, const Variant& value) const
//{
//	Variant inst(instance, VariantPolicy::NoCopy());
//
//	return SetVariantValue(inst, value);
//}

template <typename T>
T FieldInfo::GetValue(const Variant& instance) const
{
	return m_getter->GetValueReference(instance).GetValue<T>();
}

END_META_NAMESPACE