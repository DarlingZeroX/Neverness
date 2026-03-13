// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#include "pch.h"
#include "GlobalEventBus.h"
#include "Core/HLog.h"

namespace Horizon::Events {
	static 	std::vector<std::unique_ptr<HEvent>> m_EventCache;

	GlobalEventBus& GlobalEventBus::GetInstance()
	{
		static GlobalEventBus __Instance;

		return __Instance;
	}

	void GlobalEventBus::Emit(HEvent& eEvent)
	{
		H_LOG_INFO(eEvent.ToString().c_str());

		HEventDesc desc;
		desc.synchronizing = true;

		m_EventQueue.directDispatch(eEvent.GetCategoryFlags(), &eEvent, desc);
	}
	void GlobalEventBus::Enqueue(Scope<HEvent>&& eEvent)
	{
		if (m_OnProcess)
			m_HasProcessingEnqueueEvent = true;

		H_LOG_INFO(eEvent->ToString().c_str());

		auto* e = eEvent.get();
		HEventDesc desc;
		desc.synchronizing = false;

		m_EventCache.push_back(std::move(eEvent));

		m_EventQueue.enqueue(e->GetCategoryFlags(), e, desc);
	}
	void GlobalEventBus::Process()
	{
		m_OnProcess = true;
		m_EventQueue.process();
		m_OnProcess = false;

		if (m_HasProcessingEnqueueEvent == false)
		{
			m_EventCache.clear();
		}
		m_HasProcessingEnqueueEvent = false;
	}
	void GlobalEventBus::Update()
	{
		Process();
	}
}