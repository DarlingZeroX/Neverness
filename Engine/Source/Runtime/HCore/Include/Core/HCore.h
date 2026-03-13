// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include "../HConfig.h"
#include "../CoreModuleDefinitions.h"
#include "HLog.h"
#include "HAssert.h"
#include "HSTL.h"
#include "HCoreTypes.h"
#include "HObjectGlobals.h"
#include "../Math/HVector.h"
#include <stdlib.h>
#include <functional>

namespace Horizon
{
	struct CORE_MODULE_API HCore
	{
        static wstring                  GetWorkingDirectory() { return L""; }
		static wstring                  GetExecutableDirectory() { return L""; }
		static wstring                  GetMediaRootDirectory() { return HCore::GetExecutableDirectory() + L"Media\\"; }

        static string                   GetWorkingDirectoryNarrow() { return ""; }
        static string                   GetExecutableDirectoryNarrow() { return ""; }
        static string                   GetMediaRootDirectoryNarrow() { return ""; }

        static string                   GetCPUIDName() { return ""; }
	};

	template<typename T>
    class HProperty
    {
        std::function<T()> Getter;
        std::function<void(const T&)> Setter;

    public:
        HProperty(std::function<T()> gl, std::function<void(const T&)> sl) : Getter(gl), Setter(sl)
        {}

        T operator()() const
        {
            return Getter();
        }

        void operator=(const T& arg)
        {
            return Setter(arg);
        }

        operator T()
        {
            return Getter();
        }

    };

#ifndef HPROPERTY_READWRITE
#define HPROPERTY_READWRITE(T) HProperty<T>
#endif

}
