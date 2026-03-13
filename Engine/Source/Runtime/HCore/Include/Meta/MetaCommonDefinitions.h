#pragma once
#include "../CoreModuleDefinitions.h"

#define META_NAMESPACE Horizon::Meta
#define META_ATTRIBUTE META_NAMESPACE::Attribute
#define META_NOEXCEPT noexcept

#define USING_META_NAMESPACE using namespace META_NAMESPACE;

# define BEGIN_META_NAMESPACE namespace META_NAMESPACE {
# define END_META_NAMESPACE }

# define HMETA_API CORE_MODULE_API