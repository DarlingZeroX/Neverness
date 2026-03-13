// Copyright (c) 2025 梦旅缘心
// This file is part of VisionGal and is licensed under the MIT License.
// See the LICENSE file in the project root for details.

#pragma once
#include <algorithm>
#include <unordered_map>
#include <functional>
#include "../../Interface/HCoreTypes.h"
#include "../../Interface/HSTL.h"

namespace Horizon
{

	/**
	 * Handles the event flow between subscriber and publisher.
	 * @tparam Args Variable amount of parameters which can be exchanged with the delegate.
	 */
	template<class ... Args>
	class HEventDelegate {

	public:
		HEventDelegate();

		/**
		 * Subscribes a method/function to the event delegate which wants to receive the arguments specified in Args.
		 * @param handle A method/function which has the required Args as parameters
		 * @return A handle which can later be used to unsubscribe from the event delegate.
		 * @note To subscribe a method of an object to the event delegate use std::bind().
		 * If the handle is invalidated, the object will be unsubscribed automatically.
		 */
		uint32 Subscribe(std::function<void(Args ...)> handle);

		template<class _Fx, class TYPE_THIS>
		_NODISCARD inline uint32 SubscribeMethod(_Fx&& _method, TYPE_THIS&& _this)
		{
			return Subscribe(std::bind(_method, _this, std::placeholders::_1));
		}

		//template <class _Fx, class... _Types>
		//_NODISCARD _CONSTEXPR20 _Binder<_Unforced, _Fx, _Types...> bind(_Fx&& _Func, _Types&&... _Args) {
		//	return _Binder<_Unforced, _Fx, _Types...>(_STD forward<_Fx>(_Func), _STD forward<_Types>(_Args)...);
		//}
		/**
		 * Unsubscribes a method/function from the event delegate.
		 * @param subscriberID The handle which was returned previously at the time of subscription.
		 */
		void Unsubscribe(uint32 subscriberID);

		/**
		 * Publishes Args arguments to the subscribers.
		 * @param args The arguments which should be published
		 */
		void Invoke(Args ... args);

		//void operator()(Args ... args)
		//{
		//	Invoke(std::forward<Args>(args...));
		//}

		void operator()(Args ... args)
		{
			Invoke(std::forward<Args>(args)...);
		}
	private:
		std::unordered_map<uint32, std::function<void(Args ...)>> handler;
		uint32_t count;

		std::mutex mutex;
	};

	template<class... Args>
	HEventDelegate<Args...>::HEventDelegate()
		:count(0)
	{}

	template<class... Args>
	uint32_t HEventDelegate<Args...>::Subscribe(std::function<void(Args ...)> handle) {

		std::lock_guard<std::mutex> guard(mutex);

		auto id = count++;
		handler[id] = handle;
		return id;

	}

	template<class... Args>
	void HEventDelegate<Args...>::Unsubscribe(uint32_t subscriberID) {

		std::lock_guard<std::mutex> guard(mutex);
		handler.erase(subscriberID);

	}

	template<class... Args>
	void HEventDelegate<Args...>::Invoke(Args ... args) {

		std::unique_lock<std::mutex> lock(mutex);

		auto copy = std::vector<std::function<void(Args ...)>>();

		std::transform(handler.begin(), handler.end(), std::back_inserter(copy),
			[](const auto& it) -> std::function<void(Args ...)> { return it.second; });

		lock.unlock();

		for (auto& handle : copy) {
			handle(args ...);
		}

	}
}