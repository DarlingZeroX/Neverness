/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ObjectWrapper.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../MetaCommonDefinitions.h"
#include "Variant/VariantBase.h"

#include "../Deps/JsonConfig.h"

BEGIN_META_NAMESPACE
class Object;

class HMETA_API ObjectWrapper : public VariantBase
{
public:
	ObjectWrapper(Object* instance);

	Type GetType(void) const override;
	void* GetPtr(void) const override;

	int ToInt(void) const override;
	bool ToBool(void) const override;
	float ToFloat(void) const override;
	double ToDouble(void) const override;
	std::string ToString(void) const override;

	std::int8_t ToInt8() const override;
	std::int16_t ToInt16() const override;
	std::int32_t ToInt32() const override;
	std::int64_t ToInt64() const override;
	std::uint8_t ToUint8() const override;
	std::uint16_t ToUint16() const override;
	std::uint32_t ToUint32() const override;
	std::uint64_t ToUint64() const override;
	long ToLong() const override;

	VariantBase* Clone(void) const override;

	bool IsValid(void) const;

	//void OnSerialize(Json::object& output) const override;
	//void OnDeserialize(const Json& input) override;
private:
	Object* m_object;
};
END_META_NAMESPACE