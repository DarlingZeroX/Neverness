#include "pch.h"

#include "MethodInfo.h"

#include "../../Deps/Logging.h"

BEGIN_META_NAMESPACE
MethodInfo::ConstructArgs::ConstructArgs(const std::string& name, const std::vector<std::string>& paramNames) noexcept
	:methodName(name),
	paramNames(paramNames)
{
}

MethodInfo::MethodInfo(void)
	: Invokable()
	, m_isConst(true)
	, m_isStatic(true)
	, m_classType(Type::Invalid())
	, m_invoker(nullptr) { }

const MethodInfo& MethodInfo::Invalid(void)
{
	static MethodInfo invalid;

	return invalid;
}

Type MethodInfo::GetClassType(void) const
{
	return m_classType;
}

Type MethodInfo::GetReturnType() const
{
	return m_invoker->GetReturnType();
}

const std::vector<ParameterInfo>& MethodInfo::GetParameters() const noexcept
{
	return m_parameterInfo;
}

bool MethodInfo::IsPublic() const
{
	// TODO
	return false;
}

bool MethodInfo::IsPrivate() const
{
	// TODO
	return false;
}

bool MethodInfo::IsGlobal() const
{
	return m_classType == Type::Invalid();
}

bool MethodInfo::IsStatic() const noexcept
{
	return m_isStatic;
}

bool MethodInfo::IsValid(void) const noexcept
{
	return m_invoker != nullptr;
}

bool MethodInfo::IsConst(void) const noexcept
{
	return m_isConst;
}

Variant MethodInfo::Invoke(
	Variant& instance,
	ArgumentList& arguments
) const
{
#if defined(_DEBUG)

	UAssert(IsValid(),
		"Invalid method invoked."
	);

	UAssert(!(instance.IsConst() && !m_isConst),
		"Non-const method invoked on const object."
	);

	UAssert(instance.GetType() == m_classType,
		"Incompatible method invoked with instance."
	);

#endif

	return m_invoker->Invoke(instance, arguments);
}

Variant MethodInfo::InvokeVariadic(ArgumentList& arguments) const
{
#if defined(_DEBUG)

	UAssert(IsValid(), "Invalid function invocation.");

#endif

	Variant v;
	//return m_invoker->Invoke(arguments);
	return m_invoker->Invoke(v, arguments);
}

END_META_NAMESPACE