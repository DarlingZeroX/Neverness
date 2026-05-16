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

#include "HTimer.h"
#include "HSystemTimer.h"

namespace NN::Core
{
	HTimer::HTimer()
		:
		m_SecondsPerCount(0.0),
		m_DeltaTime(-1.0),
		m_BaseTime(0),
		m_PausedTime(0),
		m_StopTime(0),
		m_PrevTime(0),
		m_CurrTime(0),
		m_Stopped(false)
	{
		const uint64 countsPerSec = HSystemTimer::GetPerformanceFrequency();

		m_SecondsPerCount = 1.0 / static_cast<double>(countsPerSec);

		Reset();
	}

	// Returns the total time elapsed since Reset() was called, NOT counting any
	// time when the clock is stopped.
	float HTimer::TotalTime()const
	{
		if (m_Stopped)
		{
			// If we are stopped, do not count the time that has passed since we stopped.
			// Moreover, if we previously already had a pause, the distance
			// m_StopTime - m_BaseTime includes paused time, which we do not want to count.
			// To correct this, we can subtract the paused time from m_StopTime:
			//
			//                     |<--paused time-->|
			// ----*---------------*-----------------*------------*------------*------> time
			//  m_BaseTime       m_StopTime        startTime     m_StopTime    m_CurrTime

			return (float)(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
		}
		else
		{
			// The distance m_CurrTime - m_BaseTime includes paused time,
			// which we do not want to count.  To correct this, we can subtract
			// the paused time from m_CurrTime:
			//
			//  (m_CurrTime - m_PausedTime) - m_BaseTime
			//
			//                     |<--paused time-->|
			// ----*---------------*-----------------*------------*------> time
			//  m_BaseTime       m_StopTime        startTime     m_CurrTime

			return static_cast<float>(((m_CurrTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
		}
	}

	float HTimer::DeltaTime()const
	{
		return static_cast<float>(m_DeltaTime);
	}

	void HTimer::Reset()
	{
		const uint64 currTime = HSystemTimer::GetPerformanceCounter();

		m_CurrTime = currTime;
		m_BaseTime = currTime;
		m_PrevTime = currTime;
		m_StopTime = 0;
		m_PausedTime = 0;	// 涉及到多次Reset的话需要将其归0
		m_Stopped = false;
	}

	void HTimer::Start()
	{
		const uint64 startTime = HSystemTimer::GetPerformanceCounter();

		// Accumulate the time elapsed between stop and start pairs.
		//
		//                     |<-------d------->|
		// ----*---------------*-----------------*------------> time
		//  m_BaseTime       m_StopTime        startTime

		if (m_Stopped)
		{
			m_PausedTime += (startTime - m_StopTime);

			m_PrevTime = startTime;
			m_StopTime = 0;
			m_Stopped = false;
		}
	}

	void HTimer::Stop()
	{
		if (!m_Stopped)
		{
			const uint64 currTime = HSystemTimer::GetPerformanceCounter();

			m_StopTime = currTime;
			m_Stopped = true;
		}
	}

	void HTimer::Tick()
	{
		BeginTick();
		EndTick();
	}

	void HTimer::BeginTick()
	{
		if (m_Stopped)
		{
			m_DeltaTime = 0.0;
			return;
		}

		m_CurrTime = HSystemTimer::GetPerformanceCounter();

		// Time difference between this frame and the previous.
		m_DeltaTime = static_cast<double>(m_CurrTime - m_PrevTime) * m_SecondsPerCount;

		m_DeltaTime = std::max(0., m_DeltaTime);
	}

	void HTimer::EndTick()
	{
		// Prepare for next frame.
		m_PrevTime = m_CurrTime;
	}
}

//namespace Horizon
//{
//HTimer::HTimer()
//	: m_SecondsPerCount(0.0), m_DeltaTime(-1.0), m_BaseTime(0), m_StopTime(0),
//	m_PausedTime(0), m_PrevTime(0), m_CurrTime(0), m_Stopped(false)
//{
//	__int64 countsPerSec;
//	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
//	m_SecondsPerCount = 1.0 / (double)countsPerSec;
//}
//
//// Returns the total time elapsed since Reset() was called, NOT counting any
//// time when the clock is stopped.
//float HTimer::TotalTime()const
//{
//	// If we are stopped, do not count the time that has passed since we stopped.
//	// Moreover, if we previously already had a pause, the distance
//	// m_StopTime - m_BaseTime includes paused time, which we do not want to count.
//	// To correct this, we can subtract the paused time from m_StopTime:
//	//
//	//                     |<--paused time-->|
//	// ----*---------------*-----------------*------------*------------*------> time
//	//  m_BaseTime       m_StopTime        startTime     m_StopTime    m_CurrTime
//
//	if (m_Stopped)
//	{
//		return (float)(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
//	}
//
//	// The distance m_CurrTime - m_BaseTime includes paused time,
//	// which we do not want to count.  To correct this, we can subtract
//	// the paused time from m_CurrTime:
//	//
//	//  (m_CurrTime - m_PausedTime) - m_BaseTime
//	//
//	//                     |<--paused time-->|
//	// ----*---------------*-----------------*------------*------> time
//	//  m_BaseTime       m_StopTime        startTime     m_CurrTime
//
//	else
//	{
//		return (float)(((m_CurrTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
//	}
//}
//
//float HTimer::DeltaTime()const
//{
//	return (float)m_DeltaTime;
//}
//
//void HTimer::Reset()
//{
//	__int64 currTime;
//	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
//
//	m_CurrTime = currTime;
//	m_BaseTime = currTime;
//	m_PrevTime = currTime;
//	m_StopTime = 0;
//	m_PausedTime = 0;	// 涉及到多次Reset的话需要将其归0
//	m_Stopped = false;
//}
//
//void HTimer::Start()
//{
//	__int64 startTime;
//	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
//
//	// Accumulate the time elapsed between stop and start pairs.
//	//
//	//                     |<-------d------->|
//	// ----*---------------*-----------------*------------> time
//	//  m_BaseTime       m_StopTime        startTime
//
//	if (m_Stopped)
//	{
//		m_PausedTime += (startTime - m_StopTime);
//
//		m_PrevTime = startTime;
//		m_StopTime = 0;
//		m_Stopped = false;
//	}
//}
//
//void HTimer::Stop()
//{
//	if (!m_Stopped)
//	{
//		__int64 currTime;
//		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
//
//		m_StopTime = currTime;
//		m_Stopped = true;
//	}
//}
//
//void HTimer::Tick()
//{
//	BeginTick();
//	EndTick();
//}
//
//// Query current time, And calculate the delta time.
//void HTimer::BeginTick()
//{
//	if (m_Stopped)
//	{
//		m_DeltaTime = 0.0;
//		return;
//	}
//
//	__int64 currTime;
//	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
//	m_CurrTime = currTime;
//
//	// Time difference between this frame and the previous.
//	m_DeltaTime = (m_CurrTime - m_PrevTime) * m_SecondsPerCount;
//
//	if (m_DeltaTime < 0.0)
//	{
//		m_DeltaTime = 0.0;
//	}
//}
//
//// Update the previous time to current time.
//void HTimer::EndTick()
//{
//	// Prepare for next frame.
//	m_PrevTime = m_CurrTime;
//}
//}