/* ----------------------------------------------------------------------------
** Copyright (c) 2016 Austin Brunkhorst, All Rights Reserved.
**
** Type.h
** --------------------------------------------------------------------------*/

#pragma once
#include "MetaCommonDefinitions.h"
#include "Macros.h"

#include "Core/Type/TypeID.h"
#include "Core/Type/TypeConfig.h"
#include "Core/InvokableConfig.h"
#include "Core/Argument/ArgumentConfig.h"
#include "Deps/JsonConfig.h"

#include <string>
#include <vector>
#include <set>
#include <functional>

BEGIN_META_NAMESPACE
class Variant;
class Enum;
class ConstructorInfo;
class DestructorInfo;
class FieldInfo;
class MethodInfo;
class Argument;

class AttributeManager;

class HMETA_API Type
{
public:
	typedef std::vector<Type> List;
	typedef std::set<Type> Set;
	typedef std::function<Variant(const Variant&, const FieldInfo&)> SerializationGetterOverride;

	Type(void);
	Type(const Type& rhs);
	Type(TypeID id, bool isArray = false);

	operator bool(void) const;

	Type& operator=(const Type& rhs);

	bool operator<(const Type& rhs) const;
	bool operator>(const Type& rhs) const;
	bool operator<=(const Type& rhs) const;
	bool operator>=(const Type& rhs) const;
	bool operator==(const Type& rhs) const;
	bool operator!=(const Type& rhs) const;

	bool operator==(const type_info& rhs) const;
	bool operator!=(const type_info& rhs) const;

	/** @brief Gets an instance of an invalid type.
	 */
	static const Type& Invalid(void);

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	/** @brief Gets the internal id of the type.
	 */
	TypeID GetID(void) const;

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	/** @brief Gets all types registered in the main
	 *         reflection database.
	 */
	static List GetTypes(void);

	const type_info* GetTypeInfo(void);

	/** @brief Gets a type based on the qualified string name.
	 *  @param name Name of the type.
	 */
	//static Type GetFromName(const std::string& name);

	/** @brief Gets a type by deducing the type of an object.
	 *  @param obj Object to deduce type from.
	 */
	template<typename T>
	static Type Get(T&& obj);

	template<typename T>
	static Type Get();

	///////////////////////////////////////////////////////////////////
	/////////////////////	Global
	///////////////////////////////////////////////////////////////////

	/** @brief Gets all registered global variables.
	 */
	static std::vector<FieldInfo> GetGlobals(void);

	/** @brief Gets a global variable with the specified name.
	 *  @param name Qualified global name.
	 */
	static const FieldInfo& GetGlobal(const std::string& name);

	/** @brief Gets all registered global functions.
	 */
	static std::vector<MethodInfo> GetGlobalFunctions(void);

	/** @brief Gets a global function with the specified name, and
	 *         first available overload.
	 *  @param name Qualified global function name.
	 */
	static const MethodInfo& GetGlobalFunction(const std::string& name);

	/** @brief Gets a global function with the specified name, and
	 *         overload signature.
	 *  @param name Qualified global function name.
	 *  @param signature Signature of the global function.
	 */
	static const MethodInfo& GetGlobalFunction(const std::string& name, const InvokableSignature& signature);

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	/** @brief Compares two type lists.
	 *  @param a First list
	 *  @param b Second list
	 *  @return true if "a" is identical to "b"
	 */
	static bool ListsEqual(const List& a, const List& b);

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	/** @brief Determines if this type is valid.
	 *  @return true if the internal id is not Type::INVALID
	 */
	bool IsValid(void) const noexcept;

	/** @brief Determines if this type is primitive.
	 *  @return true if the type is among (int, bool, char, etc).
	 */
	bool IsPrimitive(void) const;

	/** @brief Determines if this type is a floating point
	 *          float, double, long double, etc.
	 *  @return true if the type is among (float, double, long double, etc).
	 */
	bool IsFloatingPoint(void) const;

	/** @brief Determines if this type is signed (unsigned int, etc).
	 *  @return true if the type is signed.
	 */
	bool IsSigned(void) const;

	/** @brief Determines if this type is an enumeration.
	 *  @return true if the type is either an enum or enum class.
	 */
	bool IsEnum(void) const;

	/** @brief Determines if this type is a pointer.
	 *  @return true if the type has any level of indirection.
	 *          ie - (int *), (int **), etc.
	 */
	bool IsPointer(void) const;

	/** @brief Determines if this type is a class.
	 *  @return true if the type is a class or struct.
	 */
	bool IsClass(void) const;

	/** @brief Determines if this type is an array type.
	 *  @return true if the type is an array type.
	 */
	bool IsArray(void) const;

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	std::string_view GetNamespace() const;

	/** @brief Gets full qualified name for the type.
	 *  @return Full qualified name of the type as it is declared.
	 *          ie - "boost::regex"
	 */
	std::string_view GetFullName(void) const;

	/** @brief Gets the human readable name for this type.
	 *  @return Qualified name of the type as it is declared.
	 *          ie - "boost::regex"
	 */
	std::string_view GetName(void) const;

	/** @brief Gets meta data for this type.
	 *  @return Meta Data Manager for this type.
	 */
	const AttributeManager& GetMeta(void) const;

	/** @brief Deconstructs the given object instance.
	 *  @param instance Variant object instance to destruct.
	 */
	void Destroy(Variant& instance) const;

	///////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////

	/** @brief Gets the decayed version of this type.
	 *  @return Type with no qualifiers like const, and no pointer.
	 *          ie - const int * -> int
	 */
	Type GetDecayedType(void) const;

	/** @brief Gets the type that this array type holds.
	*  @return Type this array type holds.
	*          ie - Array<double> -> double
	*          Non array types return itself.
	*/
	Type GetArrayType(void) const;

	///////////////////////////////////////////////////////////////////
	/////////////////////	Enum
	///////////////////////////////////////////////////////////////////

	/** @brief Gets the enumeration representing this type,
	*         assuming it's an enum type.
	*  @return Reference to the enum type in the reflection database.
	*/
	const Enum& AsEnum(void) const;

	/**
	 * \brief  Returns the names of the members of the current enumeration type.
	 * \return An array that contains the names of the members of the enumeration.
	 */
	const std::vector<std::string_view>& GetEnumNames() const;

	/**
	* \brief  Returns an array of the values of the constants in the current enumeration type.
	* \return An array that contains the values. The elements of the array are sorted by the
	*	binary values (that is, the unsigned values) of the enumeration constants.
	*/
	const std::vector<Variant>& GetEnumValues() const;

	/**
	 * \brief  Returns the name of the constant that has the specified value, for the current enumeration type.
	 * \return The name of the member of the current enumeration type that has the specified value, or null if no such constant is found.
	 */
	std::string_view GetEnumName(const Argument& value) const;

	const AttributeManager& GetEnumNameAttributes(std::string_view name) const;

	Type GetEnumUnderlyingType(void) const;

	//bool IsEnumDefined(const Argument& value) const;

	///////////////////////////////////////////////////////////////////
	/////////////////////	Derives
	///////////////////////////////////////////////////////////////////

	/** @brief Determines if this type derives from the specified
	 *         runtime type.
	 *  @param other Other class type.
	 *  @return true if both types are class types and this type
	 *          derives from "other".
	 */
	bool DerivesFrom(const Type& other) const;

	/** @brief Determines if this type derives from the specified
	 *         runtime type.
	 *  @return true if both types are class types and this type
	 *          derives from "other".
	 */
	template<typename T>
	bool DerivesFrom(void) const;

	/** @brief Gets all base classes for this type.
	 *  @return Type set of all base classes for this type.
	 */
	const Set& GetBaseClasses(void) const;

	/** @brief Gets all classes that derive from this class type.
	 *  @return Type set of all derived classes for this type.
	 */
	const Set& GetDerivedClasses(void) const;

	///////////////////////////////////////////////////////////////////
	/////////////////////	Constructor Info
	///////////////////////////////////////////////////////////////////

	/** @brief Gets all constructors for this type
	 *         assuming it's a class type.
	 *  @return Set of constructors for this type.
	 */
	std::vector<ConstructorInfo> GetConstructors(void) const;

	/** @brief Gets all dynamic constructors for this type
	 *         assuming it's a class type.
	 *  @return Set of all dynamic constructors for this type.
	 */
	std::vector<ConstructorInfo> GetDynamicConstructors(void) const;

	/** @brief Gets a constructor for this type.
	 *  @param signature Signature of the constructor.
	 *  @return Reference to the constructor with the given signature
	 *          in the reflection database.
	 */
	const ConstructorInfo& GetConstructor(
		const InvokableSignature& signature = InvokableSignature()
	) const;

	/** @brief Gets a dynamic constructor for this type with the
	 *         specified argument signature.
	 *  @param signature Signature of the dynamic constructor.
	 *  @return Reference to the dynamic constructor with the given
	 *          signature in the reflection database.
	 */
	const ConstructorInfo& GetDynamicConstructor(
		const InvokableSignature& signature = InvokableSignature()
	) const;

	/** @brief Gets the constructor for this array type.
	 *  @return Reference to the array constructor in the reflection database.
	 */
	const ConstructorInfo& GetArrayConstructor(void) const;

	///////////////////////////////////////////////////////////////////
	/////////////////////	Destructor Info
	///////////////////////////////////////////////////////////////////

	/** @brief Gets the destructor for this type assuming it's a
	 *         class type.
	 *  @return Reference to the destructor in the reflection database.
	 */
	const DestructorInfo& GetDestructor(void) const;

	///////////////////////////////////////////////////////////////////
	/////////////////////	Method
	///////////////////////////////////////////////////////////////////

	/** @brief Gets all methods for this type
	 *         assuming it's a class type.
	 *  @return Set of methods for this type.
	 */
	std::vector<MethodInfo> GetMethods(void) const;

	/** @brief Gets a method for this type.
	 *  @param name Name of the method.
	 *  @return Reference to the method in the reflection database.
	 *     If the method doesn't exist, an invalid method.
	 *     If the method exists, but has overloads, the first declared.
	 */
	const MethodInfo& GetMethod(const std::string& name) const;

	/** @brief Gets a method for this type.
	 *  @param name Name of the method.
	 *  @param signature Specific overload for this method.
	 *  @return Reference to the method in the reflection database
	 *          with the specified overload. If the specific overload
	 *          doesn't exist, an invalid method is returned.
	 */
	const MethodInfo& GetMethod(const std::string& name, const InvokableSignature& signature) const;

	///////////////////////////////////////////////////////////////////
	/////////////////////	Static Methods
	///////////////////////////////////////////////////////////////////

	/** @brief Gets all static methods for this type
	 *         assuming it's a class type.
	 *  @return Set of static methods for this type.
	 */
	std::vector<MethodInfo> GetStaticMethods(void) const;

	/** @brief Gets a static method for this type.
	 *  @param name Name of the static method.
	 *  @return Reference to the method in the reflection database.
	 *     If the method doesn't exist, an invalid function.
	 *     If the method exists, but has overloads, the first declared.
	 */
	const MethodInfo& GetStaticMethod(const std::string& name) const;

	/** @brief Gets a static method for this type.
	 *  @param name Name of the static method.
	 *  @param signature Specific overload for this method.
	 *  @return Reference to the method in the reflection database
	 *          with the specified overload. If the specific overload
	 *          doesn't exist, an invalid function is returned.
	 */
	const MethodInfo& GetStaticMethod(const std::string& name, const InvokableSignature& signature) const;

	///////////////////////////////////////////////////////////////////
	/////////////////////	Field
	///////////////////////////////////////////////////////////////////

	/** @brief Gets all fields for this type
	 *         assuming it's a class type.
	 *  @return Set of fields for this type.
	 */
	const std::vector<FieldInfo>& GetFields(void) const;

	/** @brief Gets a specific field for this type.
	 *  @param name Name of the field.
	 *  @return Reference to the field in the reflection database.
	 *          If the field doesn't exist, an invalid field.
	 */
	const FieldInfo& GetField(const std::string& name) const;

	///////////////////////////////////////////////////////////////////
	/////////////////////	Static Field
	///////////////////////////////////////////////////////////////////

	/** @brief Gets all static fields for this type
	 *         assuming it's a class type.
	 *  @return Set of static fields for this type.
	 */
	std::vector<FieldInfo> GetStaticFields(void) const;

	/** @brief Gets a specific static field for this type.
	 *  @param name Name of the static field.
	 *  @return Reference to the field in the reflection database.
	 *          If the field doesn't exist, an invalid global.
	 */
	const FieldInfo& GetStaticField(const std::string& name) const;

	///////////////////////////////////////////////////////////////////
	/////////////////////	Serialize Json
	///////////////////////////////////////////////////////////////////

	template<typename ClassType>
	static Json SerializeJson(const ClassType& instance, bool invokeHook = true);

	Json SerializeJson(const Variant& instance, bool invokeHook = true) const;
	Json SerializeJson(const Variant& instance, SerializationGetterOverride getterOverride, bool invokeHook = true) const;

	template<typename ClassType>
	static ClassType DeserializeJson(const Json& value);

	Variant DeserializeJson(const Json& value) const;
	Variant DeserializeJson(const Json& value, const ConstructorInfo& ctor) const;
	void DeserializeJson(Variant& instance, const Json& value) const;

private:
	friend class std::allocator<Type>;

	friend struct TypeData;

	friend class Variant;
	friend class Argument;
	friend class Enum;
	friend class EnumBase;
	friend class Constructor;
	friend class Destructor;
	friend class Method;
	friend class Field;
	//friend class Function;
	//friend class Global;

	TypeID m_id;
	bool m_isArray;
	const type_info* m_info;
};
END_META_NAMESPACE

#include "Impl/Type.hpp"