/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** GlobalSetter.hpp
** --------------------------------------------------------------------------*/
#pragma once

BEGIN_META_NAMESPACE
///////////////////////////////////////////////////////////////////////
// Setter from Method
///////////////////////////////////////////////////////////////////////

template<typename GlobalType>
class GlobalSetter<GlobalType, true> : public FieldSetterBase
{
public:
	typedef GlobalType(*Signature)(const GlobalType&);

	GlobalSetter(Signature function)
		: m_function(function) { }

	void SetValue(const Argument& value) override
	{
		m_function(value.GetValue<GlobalType>());
	}

private:
	Signature m_function;
};

///////////////////////////////////////////////////////////////////////
// Setter from Direct FieldInfo
///////////////////////////////////////////////////////////////////////

template<typename GlobalType>
class GlobalSetter<GlobalType, false> : public FieldSetterBase
{
public:
	GlobalSetter(GlobalType* field)
		: m_field(field) { }

	void SetValue(const Argument& value) override
	{
		const_cast<GlobalType&>(*m_field) = value.GetValue<GlobalType>();
	}

private:
	GlobalType* m_field;
};
END_META_NAMESPACE