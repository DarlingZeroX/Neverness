/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Method.hpp
** --------------------------------------------------------------------------*/

#pragma once

#include "TypeUnpacker.hpp"

BEGIN_META_NAMESPACE
template<class ClassType, typename ReturnType, typename ...ArgTypes>
MethodInfo::MethodInfo(
	const ConstructArgs& args,
	ReturnType(ClassType::* method)(ArgTypes...)
)
	: Invokable(args.methodName)
	, m_isConst(false)
	, m_isStatic(false)
	, m_classType(TypeOf(ClassType))
	, m_invoker(new MethodInvoker<ClassType, ReturnType, ArgTypes...>(method))
{
	constexpr unsigned argsTypeSize = sizeof...(ArgTypes);

	assert(argsTypeSize == args.paramNames.size());

	AddParameterInfo<ArgTypes...>(args, 0);

	TypeUnpacker<ArgTypes...>::Apply(m_signature);
}

template<class ClassType, typename ReturnType>
MethodInfo::MethodInfo(
	const ConstructArgs& args,
	ReturnType(ClassType::* method)()
)
	: Invokable(args.methodName)
	, m_isConst(false)
	, m_isStatic(false)
	, m_classType(TypeOf(ClassType))
	, m_invoker(new MethodInvoker<ClassType, ReturnType>(method))
{
	TypeUnpacker<>::Apply(m_signature);
}

template<class ClassType, typename ReturnType, typename ...ArgTypes>
MethodInfo::MethodInfo(
	const ConstructArgs& args,
	ReturnType(ClassType::* method)(ArgTypes...) const
)
	: Invokable(args.methodName)
	, m_isConst(true)
	, m_isStatic(false)
	, m_classType(TypeOf(ClassType))
	, m_invoker(new MethodInvoker<ClassType, ReturnType, ArgTypes...>(method))
{
	constexpr auto argsTypeSize = sizeof...(ArgTypes);

	assert(argsTypeSize == args.paramNames.size());

	AddParameterInfo<ArgTypes...>(args, 0);

	TypeUnpacker<ArgTypes...>::Apply(m_signature);
}

template<class ClassType, typename ReturnType>
MethodInfo::MethodInfo(
	const ConstructArgs& args,
	ReturnType(ClassType::* method)() const
)
	: Invokable(args.methodName)
	, m_isConst(false)
	, m_isStatic(false)
	, m_classType(TypeOf(ClassType))
	, m_invoker(new MethodInvoker<ClassType, ReturnType>(method))
{
	TypeUnpacker<>::Apply(m_signature);
}

template <typename ReturnType, typename ... ArgTypes>
MethodInfo::MethodInfo(
	const ConstructArgs& args,
	ReturnType(*function)(ArgTypes...),
	Type parentType
)
	: Invokable(args.methodName)
	, m_isConst(false)
	, m_isStatic(true)
	, m_classType(parentType)
	, m_invoker(new FunctionInvoker<ReturnType, ArgTypes...>(function))
{
	TypeUnpacker<ArgTypes...>::Apply(m_signature);
}

template<class FristType, class ...ArgTypes>
void MethodInfo::AddParameterInfo(const ConstructArgs& args, int index)
{
	ParameterInfo::ConstructArgs params;
	params.name = args.paramNames[index];
	params.position = index;
	params.paramType = TypeOf(FristType);

	m_parameterInfo.push_back(std::move(
		ParameterInfo{ params }
	));

	++index;

	if constexpr (sizeof...(ArgTypes) > 0)
		AddParameterInfo<ArgTypes...>(args, index);
}

///////////////////////////////////////////////////////////////////////

template<typename ...Args>
Variant MethodInfo::Invoke(Variant& instance, Args &&...args) const
{
	ArgumentList arguments{ std::forward<Args>(args)... };

	return Invoke(instance, arguments);
}

template <typename ... Args>
Variant MethodInfo::Invoke(Args&&... args) const
{
	ArgumentList arguments{ std::forward<Args>(args)... };

	return InvokeVariadic(arguments);
}

END_META_NAMESPACE