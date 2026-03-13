// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include <string>
#include <functional>
#include "../CoreModuleDefinitions.h"

namespace Horizon
{
	enum class HLogLevel: int
	{
		None,
		Error,
		Info,
		Warn,
		Critical
	};

	struct CORE_MODULE_API HCoreLogger
	{
		using LogCallback = std::function<void(HLogLevel,const std::string&)>;

		static void Error(const char* fmt, ...);
		static void Info(const char* fmt, ...);
		static void Warn(const char* fmt, ...);
		static void Critical(const char* fmt, ...);

		static void Error(const std::string& fmt, ...);
		static void Info(const std::string& fmt, ...);
		static void Warn(const std::string& fmt, ...);
		static void Critical(const std::string& fmt, ...);

		static void AddListener(LogCallback callback);
	};
}

//#ifdef _DEBUG
#ifndef H_NO_LOG

#define H_LOG_INFO(...)		Horizon::HCoreLogger::Info(__VA_ARGS__)
#define H_LOG_WARN(...)		Horizon::HCoreLogger::Warn(__VA_ARGS__)
#define H_LOG_CRITICAL(...) Horizon::HCoreLogger::Critical(__VA_ARGS__)
#define H_LOG_ERROR(...)	Horizon::HCoreLogger::Error(__VA_ARGS__)

#else
#define H_LOG_INFO(...)
#define H_LOG_WARN(...)
#define H_LOG_CRITICAL(...)
#define H_LOG_ERROR(...)
#endif
