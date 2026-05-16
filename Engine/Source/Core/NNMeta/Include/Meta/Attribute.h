#pragma once
#include "MetaCommonDefinitions.h"

#include "Core/Object.h"

#include "Macros.h"

BEGIN_META_NAMESPACE

class Attribute : public Object
{
};

template<typename AttributeType, typename ...Args>
Attribute* MetaPropertyInitializer(Args&&... args);

END_META_NAMESPACE

#pragma region Built-In Parser Attributes

#if (!defined(__REFLECTION_PARSER__) && defined(__INTELLISENSE__))

/** @brief Enables introspection of this type.
 *         You can also enable a class type by inheriting from META_NAMESPACE::Object.
 */
	class META_Enable : public META_ATTRIBUTE { };

///////////////////////////////////////////////////////////////////////////////

/** @brief Only registers this type. Does not generate class information.
 */
class META_Register : public META_ATTRIBUTE { };

///////////////////////////////////////////////////////////////////////////////

/** @brief Explicitly disables introspection of this type.
 */
class META_Disable : public META_ATTRIBUTE { };

///////////////////////////////////////////////////////////////////////////////

/** @brief Requires all methods in a class to be whitelisted by default.
 */
class META_WhiteListMethods : public META_ATTRIBUTE { };

///////////////////////////////////////////////////////////////////////////////

/** @brief Explicitly disables generation of non dynamic constructors.
 *         Only applies to class constructors.
 */
class META_DisableNonDynamic : public META_ATTRIBUTE { };

///////////////////////////////////////////////////////////////////////////////

/** @brief Uses the "WrapObject" Variant policy in dynamic constructors.
 */
class META_WrapObject : public META_ATTRIBUTE { };

///////////////////////////////////////////////////////////////////////////////

/** @brief Explicitly enables generation of pointers to class types.
 *         Applies to class types.
 */
class META_EnablePtrType : public META_ATTRIBUTE { };

///////////////////////////////////////////////////////////////////////////////

/** @brief Explicitly enables generation of constant pointers to class types.
 *         Applies to class types.
 */
class META_EnableConstPtrType : public META_ATTRIBUTE { };

///////////////////////////////////////////////////////////////////////////////

/** @brief Explicitly enables generation of the array type for this type.
 */
class META_EnableArrayType : public META_ATTRIBUTE { };

///////////////////////////////////////////////////////////////////////////////

/** @brief Defines an explicit getter for this type.
 *         Applies to Globals, Fields and Static Fields.
 */
class META_Getter : public META_ATTRIBUTE
{
public:
	/** @param getterName Name of the getter function.
	 *         The name is relative to the context.
	 *         For example, in a class the name is a public member function.
	 *         In a Global or Static Field, it's a globally accessible function.
	 */
	META_Getter(const char* getterName);
};

///////////////////////////////////////////////////////////////////////////////

/** @brief Defines an explicit setter for this type.
 *         Applies to Globals, Fields and Static Fields.
 */
class META_Setter : public META_ATTRIBUTE
{
public:
	/** @param setterName Name of the setter function.
	 *         The name is relative to the context.
	 *         For example, in a class the name is a public member function.
	 *         In a Global or Static Field, it's a globally accessible function.
	 */
	META_Setter(const char* setterName);
};

/** @brief Defines an explicit getter for this type. Parent type is not assumed
 *         Applies to Globals, Fields and Static Fields.
 */
class META_ExplicitGetter : public META_ATTRIBUTE
{
public:
	/** @param getterName Name of the getter function.
	 */
	META_ExplicitGetter(const char* getterName);
};

///////////////////////////////////////////////////////////////////////////////

/** @brief Defines an explicit setter for this type. Parent type is not assumed
 *         Applies to Globals, Fields and Static Fields.
 */
class META_ExplicitSetter : public META_ATTRIBUTE
{
public:
	/** @param setterName Name of the setter function.
	 */
	META_ExplicitSetter(const char* setterName);
};

///////////////////////////////////////////////////////////////////////////////

/** @brief Overrides the display name of a type. Only affects GetX( ) Type functions.
 *         Applies to Enums, Classes, Fields, Functions, Globals and Methods
 */
class META_DisplayName : public META_ATTRIBUTE
{
public:
	/** @param displayName Name of the setter function.
	 *         The name is relative to the context.
	 *         For example, in a class the name is a public member function.
	 *         In a Global or Static Field, it's a globally accessible function.
	 */
	META_DisplayName(const char* displayName);
};

#endif // if (!defined(__REFLECTION_PARSER__) && defined(__INTELLISENSE__))

#pragma endregion

#include "Impl/Attribute.hpp"
