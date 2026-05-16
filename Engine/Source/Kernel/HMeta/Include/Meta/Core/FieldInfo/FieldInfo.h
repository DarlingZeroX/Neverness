#pragma once
#include "../../MetaCommonDefinitions.h"
#include "../MetaContainer.h"

#include "../Variant/Variant.h"

#include "FieldGetter.h"
#include "FieldSetter.h"

#include "GlobalGetter.h"
#include "GlobalSetter.h"

BEGIN_META_NAMESPACE
class HMETA_API FieldInfo : public MetaContainer
{
public:
	struct InitArgs
	{
		std::string name;
		Type type;
		Type classType;
		bool isStatic = false;
		FieldGetterBase* getter;
		FieldSetterBase* setter;
	};

	static bool SetValue(Variant& instance, const Variant& value, const MethodInfo& setter);

	FieldInfo(void);

	FieldInfo(const FieldInfo&) = default;
	FieldInfo& operator=(const FieldInfo&) = default;

	FieldInfo(
		const std::string& name,
		Type type,
		Type classType,
		FieldGetterBase* getter,
		FieldSetterBase* setter
	);

	FieldInfo(const InitArgs& args);

	~FieldInfo() = default;

	bool operator==(const FieldInfo& right) const;
	bool operator!=(const FieldInfo& right) const;
public:
	static const FieldInfo& Invalid(void);

	bool IsPublic(void) const;
	bool IsPrivate(void) const;
	bool IsGlobal(void) const;
	bool IsStatic(void) const;
	bool IsValid(void) const;
	bool IsReadOnly(void) const;

	Type GetType(void) const;
	Type GetClassType(void) const;

	const std::string& GetName(void) const;

	Variant GetValue(const Variant& instance) const;
	Variant GetValueRef(const Variant& instance) const;

	template<typename T>
	T GetValue(const Variant& instance) const;

	//template<typename T>
	//bool SetValue(T& instance, const Variant& value) const;

	bool SetVariantValue(Variant& instance, const Variant& value) const;

	bool SetValue(const Argument& value) const;

private:
	friend struct TypeData;

	bool m_isStatic;

	Type m_type;
	Type m_classType;

	std::string m_name;

	std::shared_ptr<FieldGetterBase> m_getter;
	std::shared_ptr<FieldSetterBase> m_setter;
};

END_META_NAMESPACE

#include "../../Impl/FieldInfo.hpp"
