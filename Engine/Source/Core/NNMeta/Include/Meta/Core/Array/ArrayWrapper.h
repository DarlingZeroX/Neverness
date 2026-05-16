/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ArrayWrapper.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "ArrayWrapperBase.h"

BEGIN_META_NAMESPACE
class Argument;

class HMETA_API ArrayWrapper
{
public:
	ArrayWrapper(void);

	template<typename T>
	ArrayWrapper(Array<T>& rhs);

	template<typename T>
	ArrayWrapper(const Array<T>& rhs);

	Variant GetValue(size_t index) const;
	void SetValue(size_t index, const Argument& value);

	void Insert(size_t index, const Argument& value);
	void Remove(size_t index);

	size_t Size(void) const;

	bool IsValid(void) const;
	bool IsConst(void) const;
private:
	bool m_isConst;

	ArrayWrapperBase* m_base;
};
END_META_NAMESPACE

#include "../../Impl/ArrayWrapper.hpp"