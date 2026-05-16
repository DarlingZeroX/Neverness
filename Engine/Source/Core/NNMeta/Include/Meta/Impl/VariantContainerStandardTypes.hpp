/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** VariantConatinerStandardTypes.hpp
** --------------------------------------------------------------------------*/

#pragma once

#define DECLARE_STANDARD_VARIANT(type)                                     \
    template<>                                                             \
    class HMETA_API VariantContainer<type> : public VariantBase            \
    {                                                                      \
    public:                                                                \
        VariantContainer(const type &value);                               \
        VariantContainer(const type &&value);                              \
                                                                           \
        Type GetType(void) const override;                                 \
        void *GetPtr(void) const override;                                 \
                                                                           \
        int ToInt(void) const override;                                    \
        bool ToBool(void) const override;                                  \
        float ToFloat(void) const override;                                \
        double ToDouble(void) const override;                              \
        std::string ToString(void) const override;                         \
                                                                           \
        std::int8_t ToInt8() const override;                               \
        std::int16_t ToInt16() const override;                             \
        std::int32_t ToInt32() const override;                             \
        std::int64_t ToInt64() const override;                             \
        std::uint8_t ToUint8() const override;                             \
        std::uint16_t ToUint16() const override;                           \
        std::uint32_t ToUint32() const override;                           \
        std::uint64_t ToUint64() const override;                           \
        long ToLong() const override;                                      \
                                                                           \
        VariantBase *Clone(void) const override;                           \
                                                                           \
    private:                                                               \
        friend class Variant;                                              \
                                                                           \
        VariantContainer &operator=(const VariantContainer &rhs) = delete; \
                                                                           \
        type m_value;                                                      \
    };                                                                     \

BEGIN_META_NAMESPACE
// void is a tricky 'ol fella, and must be defined manually
template<>
class HMETA_API VariantContainer<void> : public VariantBase
{
public:
	VariantContainer(void);

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

private:
	friend class Variant;

	VariantContainer& operator=(const VariantContainer& rhs) = delete;
	
};

DECLARE_STANDARD_VARIANT(int);
DECLARE_STANDARD_VARIANT(unsigned int);
DECLARE_STANDARD_VARIANT(bool);
DECLARE_STANDARD_VARIANT(float);
DECLARE_STANDARD_VARIANT(double);
DECLARE_STANDARD_VARIANT(std::string);
END_META_NAMESPACE

#undef DECLARE_STANDARD_TYPE