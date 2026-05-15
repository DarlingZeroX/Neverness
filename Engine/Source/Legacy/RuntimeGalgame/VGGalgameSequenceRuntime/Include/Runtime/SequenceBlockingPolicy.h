/*
 * SequenceBlockingPolicy — 并行剪辑组的汇合策略
 *
 * - None：未处于并行组（或等价于单线，由调度器解释）。
 * - WaitAll：组内所有 Active 槽位均完成当前步后才前进主游标。
 * - WaitAny：任一条槽位完成当前步即可汇合（其余槽位在汇合帧被丢弃，不再 Tick）。
 */
#pragma once

#include "../../GSSExport.h"

namespace VisionGal::GalGame
{
	enum class SequenceBlockingPolicy
	{
		None,
		WaitAll,
		WaitAny
	};
}
