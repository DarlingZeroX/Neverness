#pragma once
#include <xlocmon>

#include "../../MetaCommonDefinitions.h"
#include "EnumBase.h"

BEGIN_META_NAMESPACE

/*!
 * The \ref enumeration class provides several meta information about an enum.
 *
 * A instance of an enumeration class can only be obtained from the \ref type class or the \ref property class.
 * See \ref Type::AsEnum() and \ref property::get_enumeration().
 *
 * Meta Information
 * ----------------------
 * A \ref enumeration object is lightweight and can be copied by value. However, each copy will refer to the same underlying enumeration.
 *
 * Typical Usage
 * ----------------------
 *
\code{.cpp}
  using namespace rttr;
  enum class MetaEnable() E_Alignment
  {
	 AlignLeft       = 0x0001,
	 AlignRight      = 0x0002,
	 AlignHCenter    = 0x0004,
	 AlignJustify    = 0x0008
  };

  auto enum_align = TypeOf(E_Alignment)
\endcode
 *
 * \see method, property, constructor and type
 */
	class HMETA_API Enum
{
public:
	struct HMETA_API Iterator
	{
		using iterator_category = std::input_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = std::pair<std::string_view, Variant>;
		using pointer = value_type*;
		using reference = value_type&;

		friend class Enum;
	protected:
		const Enum* m_enum;
		value_type m_value;
		unsigned m_index;
	public:
		explicit Iterator(const Enum& type, unsigned index);

		// Assignment operator
		Iterator& operator=(const Iterator& src);

		// Dereference an iterator
		const value_type* operator->() const;

		const value_type& operator*() const;

		// Prefix increment operator
		Iterator& operator++();

		// Postfix increment operator
		Iterator operator++(int);

		// Comparisons
		bool operator==(const Iterator& iter) const META_NOEXCEPT { return m_index == iter.m_index; }
		bool operator!=(const Iterator& iter) const META_NOEXCEPT { return m_index != iter.m_index; }
	};
public:
	~Enum() META_NOEXCEPT = default;

	const Iterator begin() const;
	const Iterator end() const;

	/*!
	* \brief Returns true if this \ref enumeration is valid, otherwise false.
	*
	* \return True if this \ref enumeration is valid, otherwise false.
	*/
	bool IsValid(void) const META_NOEXCEPT;

	/*!
	* \brief Returns true if this enumeration is the same like the \p other.
	*
	* \return True if both enumerations are equal, otherwise false.
	*/
	bool operator ==(const Enum& rhs) const META_NOEXCEPT;

	/*!
	* Returns true if this enumeration is the not the same like the \p other.
	*
	* \return True if both enumerations are different, otherwise false.
	*/
	bool operator !=(const Enum& rhs) const META_NOEXCEPT;

	/*!
	* \brief Returns the declared name of this \ref enumeration.
	*
	* \return Name of the \ref enumeration.
	*/
	const std::string_view& GetName(void) const META_NOEXCEPT;

	/*!
	* \brief Returns the type object of this \ref enumeration.
	*
	* \return Data type of the \ref enumeration.
	*/
	Type GetType(void) const;

	Type GetParentType(void) const;

	/*!
	* \brief Returns the underlying type (int, unsigned int, etc.) of this \ref enumeration.
	*
	* \return Data type of the \ref enumeration.
	*/
	Type GetUnderlyingType(void) const;

	/*!
	* \brief Returns all enum names registered for this enumeration.
	*
	* \remark When the enumeration is invalid then an empty vector is returned.
	*
	* \return A vector of enumeration names.
	*/
	const std::vector<std::string_view>& GetNames(void) const META_NOEXCEPT;

	/*!
	* \brief Returns all enum values registered for this enumeration.
	*
	* \remark When the enumeration is invalid then an empty vector is returned.
	*
	* \return A vector of enumeration values.
	*/
	const std::vector<Variant>& GetValues(void) const;

	/*!
	* \brief Returns the string_view that is used as the name of the given enumeration \p value,
	*        or an empty string_view if the \p value is not defined.
	*
	* \return A string_view object, containing the name for the given value.
	*/
	std::string_view GetName(const Argument& value) const;

	/*!
	* \brief Returns the value of the given enumeration \p name, or an empty variant if the name is not defined.
	*
	* \return A variant object, containing the value for the given \p name.
	*/
	Variant GetValue(const std::string& name) const;

	/*!
	* \brief Returns the meta data for the given name \p name.
	*
	* \return A AttributeManager object, containing attributes data.
	*/
	const AttributeManager& GetNameAttributes(const std::string& name) const;

	const Variant& operator[](const std::string& name) const;

	const std::string_view& operator()(const Argument& value) const;
private:
	friend struct TypeData;

	Enum(const EnumBase* base) META_NOEXCEPT;

	std::shared_ptr<const EnumBase> m_pContainer;
};

END_META_NAMESPACE