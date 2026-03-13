#pragma once
#include "../../MetaCommonDefinitions.h"
#include "../../Type.h"

#include <string>
#include <vector>

BEGIN_META_NAMESPACE

class Variant;
class Argument;

class HMETA_API EnumBase
{
public:
	EnumBase(std::string_view name, TypeID owner);

	virtual ~EnumBase(void) = default;

	virtual Type GetType(void) const META_NOEXCEPT = 0;
	virtual Type GetUnderlyingType(void) const META_NOEXCEPT = 0;

	virtual const std::vector<std::string_view>& GetKeys(void) const META_NOEXCEPT = 0;
	virtual const std::vector<Variant>& GetValues(void) const META_NOEXCEPT = 0;

	virtual std::string_view GetKey(const Argument& value) const META_NOEXCEPT = 0;
	virtual Variant GetValue(const std::string& key) const META_NOEXCEPT = 0;

	virtual const AttributeManager& GetNameAttributes(std::string_view name) const = 0;

	Type GetParentType(void) const;

	std::string_view GetName(void) const META_NOEXCEPT;

	virtual bool IsValid() const META_NOEXCEPT = 0;

private:
	Type m_parentType;

	std::string_view m_name;
};

class HMETA_API NullEnumBase : public EnumBase
{
public:
	NullEnumBase();

	~NullEnumBase() override = default;
public:
	Type GetType() const META_NOEXCEPT override;

	Type GetUnderlyingType() const META_NOEXCEPT override;

	const std::vector<std::string_view>& GetKeys() const META_NOEXCEPT override;

	const std::vector<Variant>& GetValues() const META_NOEXCEPT override;

	std::string_view GetKey(const Argument& value) const META_NOEXCEPT override;

	Variant GetValue(const std::string& key) const META_NOEXCEPT override;

	const AttributeManager& GetNameAttributes(std::string_view name) const override;

	bool IsValid() const META_NOEXCEPT override;

	static std::shared_ptr<NullEnumBase> Instance();
};

END_META_NAMESPACE