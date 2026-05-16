/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Argument.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "../Type/TypeConfig.h"

BEGIN_META_NAMESPACE
class Type;
class Variant;

class HMETA_API Argument
{
public:
	Argument(void) META_NOEXCEPT;
	Argument(const Argument& rhs);

	Argument(Variant& obj);
	Argument(const Variant& obj);

	template<typename T>
	Argument(const T& data);

	template<typename T>
	Argument(T& data);

	Argument& operator=(const Argument& rhs);
public:
	bool operator==(const Argument& rhs) const;
	bool operator!=(const Argument& rhs) const;

	Type GetType(void) const;

	void* GetPtr(void) const META_NOEXCEPT;

	template<typename T>
	T& GetValue(void) const;

private:
	const TypeID m_typeID;
	const bool m_isArray;

	const void* m_data;
};
END_META_NAMESPACE

#include "../../Impl/Argument.hpp"