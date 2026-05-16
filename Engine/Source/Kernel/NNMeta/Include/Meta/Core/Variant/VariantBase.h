#pragma once
#include "../../MetaCommonDefinitions.h"
#include <string>

#include "../../Deps/JsonConfig.h"

BEGIN_META_NAMESPACE

class Type;
class ArrayWrapper;

class HMETA_API VariantBase
{
public:
	virtual ~VariantBase(void) = default;

	virtual Type GetType(void) const = 0;

	virtual void* GetPtr(void) const = 0;

	virtual std::int8_t ToInt8(void) const = 0;
	virtual std::int16_t ToInt16(void) const = 0;
	virtual std::int32_t ToInt32(void) const = 0;
	virtual std::int64_t ToInt64(void) const = 0;
	virtual std::uint8_t ToUint8(void) const = 0;
	virtual std::uint16_t ToUint16(void) const = 0;
	virtual std::uint32_t ToUint32(void) const = 0;
	virtual std::uint64_t ToUint64(void) const = 0;

	virtual int ToInt(void) const = 0;
	virtual long ToLong(void) const = 0;
	virtual bool ToBool(void) const = 0;
	virtual float ToFloat(void) const = 0;
	virtual double ToDouble(void) const = 0;
	virtual std::string ToString(void) const = 0;

	//virtual bool IsValid(void) const = 0;
	virtual bool IsArray(void) const;
	virtual ArrayWrapper GetArray(void) const;

	virtual VariantBase* Clone(void) const = 0;

	virtual void OnSerialize(Json::object& output) const { }
	virtual void OnDeserialize(const Json& input) { }
};

class HMETA_API NullVariantBase : public VariantBase
{
public:
	~NullVariantBase(void) override = default;

	Type GetType() const override;

	void* GetPtr() const override;

	std::int8_t ToInt8() const override;
	std::int16_t ToInt16() const override;
	std::int32_t ToInt32() const override;
	std::int64_t ToInt64() const override;
	std::uint8_t ToUint8() const override;
	std::uint16_t ToUint16() const override;
	std::uint32_t ToUint32() const override;
	std::uint64_t ToUint64() const override;

	int ToInt() const override;
	long ToLong() const override;
	bool ToBool() const override;
	float ToFloat() const override;
	double ToDouble() const override;
	std::string ToString() const override;

	VariantBase* Clone() const override;

	static NullVariantBase* Instance();
	bool IsValid() const;
	
};

END_META_NAMESPACE