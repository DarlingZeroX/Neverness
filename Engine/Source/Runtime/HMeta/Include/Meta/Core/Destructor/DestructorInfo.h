#pragma once
#include "../../MetaCommonDefinitions.h"
#include "../Invokable.h"

#include "DestructorInvoker.h"

BEGIN_META_NAMESPACE

/*!
 * The \ref destructor class provides a destructor for registered types.
 *
 * A instance of a destructor class can only be obtained from the \ref type class.
 * See \ref type::get_destructor().
 *
 * Copying and Assignment
 * ----------------------
 * A \ref destructor object is lightweight and can be copied by value. However, each copy will refer to the same underlying destructor.
 *
 * Typical Usage
 * ----------------------
 *
 * \code{.cpp}
 *     constructor string_ctor == type::get_by_name("std::string").get_constructor({type::get<const char*>()});
 *
 *     variant my_string = string_ctor.invoke("Hello World"); // returns an ptr to the object on the heap
 *
 *     type::get("std::string").get_destructor().invoke(my_string);
 *
 *     my_string.is_valid(); // yield to false
 * \endcode
 *
 * \see method, property, enumeration, constructor and type
 */
	class HMETA_API DestructorInfo : public Invokable
{
public:
	DestructorInfo(void);
	DestructorInfo(Type classType, DestructorInvokerBase* invoker);

public:
	static const DestructorInfo& Invalid(void) META_NOEXCEPT;

	/*!
	* Returns the class that declares this destructor.
	*
	* \remark When this destructor is not valid, this function will return an invalid type object (see \ref type::is_valid).
	*
	* \return \ref type "Type" of the declaring class/struct for this destructor.
	*/
	Type GetClassType(void) const META_NOEXCEPT;

	/*!
	* \brief Returns true whether this destructor object is valid; otherwise false.
	*
	* \return Returns true when the destructor is valid; otherwise false.
	*/
	bool IsValid(void) const META_NOEXCEPT;

	/*!
	* \brief Destroys the contained object in the variant \p obj.
	*
	* \remark When the \p obj could be destroyed the given \p obj is invalid after calling this method;
	*         Otherwise it is still valid.
	*
	* \return True if the destructor of the instance could be invoked, otherwise false.
	*/
	bool Invoke(Variant& obj) const;

	/*!
	* \brief Returns true if this destructor is the same like the \p other.
	*
	* \return True if both destructors are equal, otherwise false.
	*/
	bool operator==(const DestructorInfo& rhs) const META_NOEXCEPT;

	/*!
	 * Returns true if this destructor is the not the same like the \p other.
	 *
	 * \return True if both destructors are different, otherwise false.
	 */
	bool operator!=(const DestructorInfo& rhs) const META_NOEXCEPT;

private:
	Type m_classType;

	std::shared_ptr<DestructorInvokerBase> m_invoker;
};

END_META_NAMESPACE