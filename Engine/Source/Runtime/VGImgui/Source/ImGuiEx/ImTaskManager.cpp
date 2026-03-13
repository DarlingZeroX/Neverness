#include "pch.h"
#include "ImGuiEx/ImTaskManager.h"

namespace ImGuiEx {
	void ImTaskManager::CreateManager()
	{
		new ImTaskManager();
	}

	std::shared_ptr<ImTaskInterface::Task> ImTaskManager::NewTask(
		ImTaskInterface* task,
		const std::string& taskName, ImTaskFlags flags
	)
	{
		auto taskInternal = CreateRef<TaskInternal>(taskName, flags, task);
		m_currentUITasks.emplace_back(taskInternal);

		return m_currentUITasks.back();
	}

	void ImTaskManager::RenderUITask()
	{
		size_t index = 0;

		for(auto begin = m_currentUITasks.begin(), end = m_currentUITasks.end(); begin != end; )
		{
			auto& task = *begin;

			task->Context.Index = index;
			task->TaskInstance->RenderUI(task->Context);

			if (task->Context.IsFinished == true)
			{
				const auto item = begin++;
				m_currentUITasks.erase(item);
				continue;
			}

			++begin;
			++index;
		}
	}

	void ImTaskManager::FrameUpdate()
	{
		for (auto begin = m_currentUITasks.begin(), end = m_currentUITasks.end(); begin != end; )
		{
			auto& task = *begin;

			task->TaskInstance->FrameUpdate();

			++begin;
		}
	}

	void ImTaskManager::FixedUpdate()
	{
		for (auto begin = m_currentUITasks.begin(), end = m_currentUITasks.end(); begin != end; )
		{
			auto& task = *begin;

			task->TaskInstance->FixedUpdate();

			++begin;
		}
	}

	Ref<ImTaskInterface::Task> NewUITask(ImTaskInterface* task, const std::string& taskName, ImTaskFlags flags)
	{
		return ImTaskManager::GetInstance().NewTask(
			task,
			taskName,
			flags
		);
	}
}
