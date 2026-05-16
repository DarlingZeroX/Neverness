/*
 * This source file is part of VisionGal, the Visual Novel Engine
 *
 * For the latest information, see https://darlingzerox.github.io/VisionGalDoc/
 * GitHub page: https://github.com/DarlingZeroX/VisionGal
 *
 * Copyright (c) 2025-present 梦旅缘心
 *
 * See the LICENSE file in the project root for details.
 */

#pragma once
#include "../HCorePlatformConfig.h"
#include <HCore/Interface/HCoreTypes.h>

namespace Horizon
{
	class H_CORE_PLATFORM_API HTimer
	{
	public:
		HTimer();
		HTimer(const HTimer&) = default;
		HTimer& operator=(const HTimer&) = default;
		~HTimer() = default;
	public:
		float TotalTime() const;
		float DeltaTime() const;

		// 在消息循环之前调用
		void Reset();

		// 在每一帧的时候调用
		void Tick();

		// Query current time, and calculate the delta time.
		void BeginTick();

		// Update the previous time to current time.
		void EndTick();

		// 在取消暂停的时候调用
		void Start();

		// 在暂停的时候调用
		void Stop();
	private:
		double m_SecondsPerCount;
		double m_DeltaTime;

		uint64 m_BaseTime;
		uint64 m_PausedTime;
		uint64 m_StopTime;
		uint64 m_PrevTime;
		uint64 m_CurrTime;

		bool m_Stopped;
	};
}