/*
 * SequenceSignalBus — 实现
 */

#include "Runtime/SequenceSignalBus.h"

namespace VisionGal::GalGame
{
	void SequenceSignalBus::Emit(SequenceSignal signal)
	{
		m_Queue.push_back(std::move(signal));
	}

	void SequenceSignalBus::Subscribe(const std::string& name, std::function<void(const SequenceSignal&)> fn)
	{
		if (!fn)
			return;
		m_Subscribers[name].push_back(std::move(fn));
	}

	void SequenceSignalBus::DispatchQueued()
	{
		std::vector<SequenceSignal> batch;
		batch.swap(m_Queue);
		for (const SequenceSignal& sig : batch)
		{
			const auto it = m_Subscribers.find(sig.Name);
			if (it == m_Subscribers.end())
				continue;
			for (const auto& cb : it->second)
			{
				if (cb)
					cb(sig);
			}
		}
	}

	void SequenceSignalBus::ClearSubscribers() noexcept
	{
		m_Subscribers.clear();
	}
}
