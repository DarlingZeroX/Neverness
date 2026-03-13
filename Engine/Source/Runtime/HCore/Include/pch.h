#pragma once
#ifndef __CORE_MODULE_PCH_H__
#define __CORE_MODULE_PCH_H__

#pragma warning(disable : 26812)
// 26812: The enum type 'x' is unscoped. Prefer 'enum class' over 'enum' (Enum.3).

// standard library
#include <array>
#include <string>
#include <set>
#include <list>
#include <map>
#include <queue>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <cstddef>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <cassert>

#include "Core/HAssert.h"

#include <memory>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <iostream>
#include <random>
#include <functional>
#include <optional>

#include <atomic>
#include <mutex>

#include "Meta/MetaCommonDefinitions.h"
#include "Meta/Meta.h"
#include "HConfig.h"
#include "Math/HVector.h"

//#if PLATFORM_WIN32
//#pragma warning(push)
//#pragma warning(disable : 4005)
//#define NOMINMAX
//#define NODRAWTEXT
////#define NOGDI
//#define NOBITMAP
//#define NOMCX
//#define NOSERVICE
//#define NOHELP
//#pragma warning(pop)
//
//#include <Windows.h>
//#endif

#endif //__CORE_MODULE_PCH_H__
