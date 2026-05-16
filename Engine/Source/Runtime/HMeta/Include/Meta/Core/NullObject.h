#pragma once
#include <string>
#include <vector>
#include "Variant/Variant.h"

BEGIN_META_NAMESPACE
namespace NullObject
{
	static std::string NullString;
	static std::string_view NullStringView;
	static std::vector<std::string> NullStringVector;
	static std::vector<std::string_view> NullStringViewVector;

	static Variant NullVariant;
	static std::vector<Variant> NullVariantVector;
}

END_META_NAMESPACE