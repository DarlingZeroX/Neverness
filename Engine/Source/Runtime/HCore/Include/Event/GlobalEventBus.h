// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include <vector>
#include <sstream>

#include "HEventBase.h"
#include "../CoreModuleDefinitions.h"
#include "../HConfig.h"
#include "Eventpp/eventqueue.h"
#include <vector>

namespace Horizon::Events {
	using GlobalEventCallback = void(const HEvent*, const HEventDesc);
	//using GlobalEventCallback = std::function<void(const HEvent*, const HEventDesc)>;

	class CORE_MODULE_API GlobalEventBus
	{
		GlobalEventBus() = default;
	public:
		~GlobalEventBus() = default;

		static GlobalEventBus& GetInstance();

		void Emit(HEvent& eEvent);
		void Enqueue(Scope<HEvent>&& eEvent);
		void Process();
		void Update();

		template<typename T>
		static	void AppendGlobalEventListener(GlobalEventCategory category, const T callback)
		{
			GetInstance().m_EventQueue.appendListener(category, callback);
		}
		
		//static	void AppendGlobalEventListener(GlobalEventCategory category, const T callback)
		//{
		//	GetInstance().m_EventQueue.appendListener(category, callback);
		//}
	private:
		bool m_OnProcess = false;
		bool m_HasProcessingEnqueueEvent = false;
		eventpp::EventQueue<GlobalEventCategory, GlobalEventCallback> m_EventQueue;
	};

	//#define ADD_GLOBAL_EVENT_LISTENER(category,callback) \
	//	Horizon::GlobalEventBus::GetInstance().AppendListener(category,	\
	//	[](const Horizon::Event* eEvent, const Horizon::EventDesc eDesc)	\
	//	{	\
	//		callback(eEvent, eDesc);		\
	//	});
	//

#define TRIGGER_GLOBAL_EVENT(e) Events::GlobalEventBus::GetInstance().Emit(e);
#define TRIGGER_GLOBAL_EVENT_DEFERRED(e) Events::GlobalEventBus::GetInstance().Enqueue(e);

#define ADD_CLASS_GLOBAL_EVENT_LISTENER(category,callback) \
		Horizon::Events::GlobalEventBus::AppendGlobalEventListener(category,	\
		[this](const Events::HEvent* eEvent, const Events::HEventDesc eDesc)	\
		{	\
			this->callback(eEvent, eDesc);		\
		});
}