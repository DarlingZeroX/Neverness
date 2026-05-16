#pragma once
#include "../../MetaCommonDefinitions.h"

BEGIN_META_NAMESPACE

namespace VariantPolicy
{
	/** @brief Wraps an Object pointer, rather than copying the value.
	 *         This also ensures base class pointers can resolve their inherited type
	 */
	struct WrapObject { };

	/** @brief Uses a reference to a value rather than copying it
	 */
	struct NoCopy { };
}

END_META_NAMESPACE