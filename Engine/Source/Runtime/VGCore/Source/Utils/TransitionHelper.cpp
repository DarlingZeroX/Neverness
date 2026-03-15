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

#include "Utils/TransitionHelper.h"

namespace VisionGal
{
	TransitionHelper::TransitionHelper()
		:callback(nullptr)
	{
		Reset();;
	}

	TransitionHelper::TransitionHelper(float durationSeconds, std::function<void()> onComplete)
		: duration(durationSeconds), callback(onComplete) {
		Reset();
	}

	void TransitionHelper::Reset()
	{
		startTime = std::chrono::high_resolution_clock::now();
		progress = 0.f;
	}

	float TransitionHelper::Update()
	{
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> elapsed = now - startTime;
		progress = std::min(elapsed.count() / duration, 1.0f);

		if (progress >= 1.0f && callback) {
			callback();  // 转场完成时触发回调
			callback = nullptr;  // 避免重复调用
		}

		return progress;
	}

	bool TransitionHelper::IsFinished() const
	{
		return progress >= 1.0f;
	}

	void TransitionHelper::Finish()
	{
		progress = 1.0f;
	}

	float TransitionHelper::Progress() const
	{
		return progress;
	}

}