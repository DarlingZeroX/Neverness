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

#include "UIDocument.h"

#include "NNRuntimeCore/Interface/EngineState.h"
//#include "Engine/Manager/SceneManager.h"

namespace NN::Runtime
{
	RmlUIDocument::RmlUIDocument()
	{
	}

	RmlUIDocument::~RmlUIDocument()
	{
		Close();
	}

	void RmlUIDocument::Close()
	{
		m_LuaUpdateCallbacks.clear();

		if (document && !isClosed)
		{
			document->Close();
			isClosed = true;
		}
	}

	void RmlUIDocument::AddUpdateCallback(const sol::function& callback)
	{
		//if (SceneManager::Get()->IsPlayMode() == false)
		//	return;
		if (EngineRuntimeState::Get().isPlayMode == false)
			return;

		m_LuaUpdateCallbacks.push_back(callback);
	}

	void RmlUIDocument::AddTimerCallback(float interval, const sol::function& callback)
	{
		//if (SceneManager::Get()->IsPlayMode() == false)
		//	return;
		if (EngineRuntimeState::Get().isPlayMode == false)
			return;

		TimerCallback item;
		item.timer = MakeRef<TransitionHelper>(interval);
		item.callback = callback;
		item.interval = interval;

		// 不能直接加入 m_LuaTimerCallbacks，因为可能正在遍历它
		m_LuaTimerUpdateAddedCallbacks.push_back(item);
	}

	// 计划（伪代码）
// 1. 保持对原有 Update 的显示回调逻辑不变
// 2. 对定时器回调使用两步策略：
//    a. 遍历当前 m_LuaTimerCallbacks，调用每个 timer.Update()
//    b. 如果 timer 已完成，则调用其 Lua 回调并调用 timer.Finish()，不将该项保留
//    c. 如果 timer 未完成，则将该项移动到一个临时容器 remaining 中
// 3. 遍历结束后，用 remaining 替换 m_LuaTimerCallbacks，从而移除已完成的回调项
// 这样可以安全地在遍历过程中删除已完成的定时器项并保持剩余项的顺序

	void RmlUIDocument::Update()
	{
		//if (SceneManager::Get()->IsPlayMode() == false)
		//	return;
		if (EngineRuntimeState::Get().isPlayMode == false)
			return;

		// 调用显示回调
		for (auto& callback : m_LuaUpdateCallbacks)
		{
			sol::protected_function_result res = callback();
			if (!res.valid()) {
				sol::error err = res;
				H_LOG_ERROR("%s", err.what());
			}
		}

		// 调用定时器回调并移除已完成的项
		{
			std::vector<TimerCallback> remaining;
			remaining.reserve(m_LuaTimerCallbacks.size());
		
			for (auto& entry : m_LuaTimerCallbacks)
			{
				auto& timer = entry.timer;
				auto& callback = entry.callback;
		
				float progress = timer->Update();
				(void)progress;
		
				if (timer->IsFinished())
				{
					sol::protected_function_result res = callback();
					if (!res.valid()) {
						sol::error err = res;
						H_LOG_ERROR("%s", err.what());
					}
					timer->Finish();
					// 不将已完成的项加入 remaining，从而实现移除
				}
				else
				{
					// 将未完成的项移动到 remaining，避免额外拷贝
					remaining.push_back(std::move(entry));
				}
			}
		
			// 用剩余的未完成定时器替换原容器，已完成项被移除
			m_LuaTimerCallbacks = std::move(remaining);
			
			// 不能直接加入 m_LuaTimerCallbacks，因为可能正在遍历它，在此处加入新添加的定时器
			m_LuaTimerCallbacks.insert(m_LuaTimerCallbacks.end(), m_LuaTimerUpdateAddedCallbacks.begin(), m_LuaTimerUpdateAddedCallbacks.end());
			m_LuaTimerUpdateAddedCallbacks.clear();
		}
	}
}
