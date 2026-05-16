/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** GlobalGetter.hpp
** --------------------------------------------------------------------------*/

#pragma once

BEGIN_META_NAMESPACE
///////////////////////////////////////////////////////////////////////
// Getter from Method
///////////////////////////////////////////////////////////////////////

template<typename GlobalType>
class GlobalGetter<GlobalType, true> : public FieldGetterBase
{
public:
	typedef GlobalType(*Signature)(void);

	GlobalGetter(Signature function)
		: m_function(function) { }

	Variant GetValue(const Variant& obj) override
	{
		return Variant{ (m_function)(), VariantPolicy::NoCopy() };
	}
private:
	Signature m_function;
};

///////////////////////////////////////////////////////////////////////
// Getter from Direct Global
///////////////////////////////////////////////////////////////////////

template<typename GlobalType>
class GlobalGetter<GlobalType, false> : public FieldGetterBase
{
public:
	GlobalGetter(GlobalType* global)
		: m_global(global) { }

	Variant GetValue(const Variant& obj) override
	{
		return *m_global;
	}

	Variant GetValueReference(const Variant& obj) override
	{
		return Variant{ *m_global, VariantPolicy::NoCopy() };
	}

private:
	GlobalType* m_global;
};
END_META_NAMESPACE