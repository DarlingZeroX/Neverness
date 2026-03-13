/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** ConstructorInfo.h
** --------------------------------------------------------------------------*/

#pragma once
#include "../../MetaCommonDefinitions.h"
#include "../MetaContainer.h"
#include "../Invokable.h"

#include "../../Type.h"

#include "ConstructorInvoker.h"

#include <memory>

BEGIN_META_NAMESPACE
class Variant;
class Argument;

class HMETA_API ConstructorInfo : public MetaContainer, public Invokable
{
public:
	ConstructorInfo(void);
	ConstructorInfo(const ConstructorInfo& rhs);
	ConstructorInfo(const ConstructorInfo&& rhs) META_NOEXCEPT;

	ConstructorInfo(
		Type classType,
		InvokableSignature signature,
		ConstructorInvokerBase* invoker,
		bool isDynamic
	);

	ConstructorInfo& operator=(const ConstructorInfo& rhs);

	ConstructorInfo& operator=(const ConstructorInfo&& rhs);

public:
	static const ConstructorInfo& Invalid(void);

	/*!
	 * \brief Returns the \ref type of the class or struct that declares this \ref constructor.
	 *
	 * \remark When this constructor does not belong to a class (i.e. is a primitive type) it will return an invalid type object.
	 *         When this constructor is not valid, this function will return an invalid type object (see \ref type::is_valid).
	 *
	 * \return \ref type "Type" of the declaring class/struct for this enumeration.
	 */
	Type GetClassType(void) const;

	/*!
	* \brief Returns true if this constructor is valid, otherwise false.
	*
	* \return True if this constructor is valid, otherwise false.
	*/
	bool IsValid(void) const;

	bool IsDynamic(void) const;

	/*!
	* \brief Invokes the constructor of type returned by \ref get_instantiated_type().
	*        The instance will always be created on the heap and will be returned as variant object.
	*         Use this method when you need to instantiate a constructor with more then 6 parameters.
	*
	* \remark The given argument type has to match **exactly** the type of the underling constructor parameter,
	*         otherwise the constructor cannot be invoked and an invalid \ref variant object (see \ref variant::is_valid)
	*         will be returned.
	*         Using this invoke function is slower, then specifying the arguments directly.
	*
	* \see get_parameter_infos()
	*
	* \return An instance of the type \ref get_instantiated_type().
	*/
	Variant InvokeVariadic(const ArgumentList& arguments) const;

	/*!
	* \brief Invokes the constructor of type returned by \ref get_instantiated_type().
	*        The instance will always be created on the heap and will be returned as variant object.
	*
	* \remark The given argument type has to match **exactly** the type of the underling constructor parameter,
	*         otherwise the constructor cannot be invoked and an invalid \ref variant object (see \ref variant::is_valid)
	*         will be returned.
	*
	* \see get_parameter_infos()
	*
	* \return An instance of the type \ref get_instantiated_type().
	*/
	template<typename ...Args>
	Variant Invoke(Args &&...args) const;

	/*!
	* \brief Returns true if this constructor is the same like the \p other.
	*
	* \return True if both constructors are equal, otherwise false.
	*/
	bool operator==(const ConstructorInfo& rhs) const META_NOEXCEPT;

	/*!
	 * Returns true if this constructor is the not the same like the \p other.
	 *
	 * \return True if both constructors are different, otherwise false.
	 */
	bool operator!=(const ConstructorInfo& rhs) const META_NOEXCEPT;
private:
	bool m_isDynamic;

	Type m_classType;

	std::shared_ptr<ConstructorInvokerBase> m_invoker;
};

END_META_NAMESPACE

#include "../../Impl/ConstructorInfo.hpp"
