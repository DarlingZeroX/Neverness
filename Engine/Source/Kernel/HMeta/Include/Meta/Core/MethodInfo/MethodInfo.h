#pragma once
#include "../../MetaCommonDefinitions.h"
#include "../MetaContainer.h"

#include "../Invokable.h"

#include "../Variant/Variant.h"
#include "../Argument/Argument.h"
#include "../ParameterInfo/ParameterInfo.h"

#include "MethodInvoker.h"
#include "FunctionInvoker.h"

#include <vector>
#include <unordered_map>
#include <functional>

BEGIN_META_NAMESPACE
class HMETA_API MethodInfo : public MetaContainer, public Invokable
{
public:
	struct HMETA_API ConstructArgs
	{
		ConstructArgs(const std::string& name, const std::vector<std::string>& paramNames) noexcept;

		const std::string& methodName;
		const std::vector<std::string>& paramNames;
	};
public:
	MethodInfo(void);

	template<class ClassType, typename ReturnType, typename ...ArgTypes>
	MethodInfo(
		const ConstructArgs& args,
		ReturnType(ClassType::* method)(ArgTypes...)
	);

	template<class ClassType, typename ReturnType>
	MethodInfo(
		const ConstructArgs& args,
		ReturnType(ClassType::* method)()
	);

	// detect const-ness
	template<class ClassType, typename ReturnType, typename ...ArgTypes>
	MethodInfo(
		const ConstructArgs& args,
		ReturnType(ClassType::* method)(ArgTypes...) const
	);

	template<class ClassType, typename ReturnType>
	MethodInfo(
		const ConstructArgs& args,
		ReturnType(ClassType::* method)() const
	);

	/**
	 * \brief Static method Constructor
	 */
	template<typename ReturnType, typename ...ArgTypes>
	MethodInfo(
		const ConstructArgs& args,
		ReturnType(*function)(ArgTypes...),
		Type parentType = Type::Invalid()
	);

private:
	template<class FristType, class ...ArgTypes>
	void AddParameterInfo(const ConstructArgs& args, int index);
public:
	static const MethodInfo& Invalid(void);

	bool IsPublic(void) const;
	bool IsPrivate(void) const;
	bool IsGlobal(void) const;
	bool IsStatic(void) const noexcept;
	bool IsValid(void) const noexcept;
	bool IsConst(void) const noexcept;

	Type GetClassType(void) const;
	Type GetReturnType(void) const;

	const std::vector<ParameterInfo>& GetParameters() const noexcept;

public:
	Variant Invoke(Variant& instance, ArgumentList& arguments) const;

	template<typename ...Args>
	Variant Invoke(Variant& instance, Args &&...args) const;

	Variant InvokeVariadic(ArgumentList& arguments) const;

	template<typename ...Args>
	Variant Invoke(Args &&...args) const;
private:
	bool m_isConst;

	bool m_isStatic;

	Type m_classType;

	std::shared_ptr<MethodInvokerBase> m_invoker;

	std::vector<ParameterInfo> m_parameterInfo;
};
END_META_NAMESPACE

#include "../../Impl/MethodInfo.hpp"
