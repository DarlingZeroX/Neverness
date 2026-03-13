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
#include "../imconfig.h"
#include "ImTaskInterface.h"
#include <stdint.h>
#include <HCore/Interface/HConfig.h>
#include <HCore/Interface/HSingleton.h>

namespace ImGuiEx {

    // creation flags
    enum class ImTaskFlags : std::uint32_t
    {
        None = 0,
        ShowInUI = (1 << 0),
        UseThreadPool = (1 << 1),                     // this will spawn the task using a limited pool of threads (with somewhere between 'cores-1' and 'logical threads-1' thread count) which means the thread might have to wait before it will start running so be careful not to cause deadlocks with any internal dependencies 
    };

	class IMGUI_API ImTaskManager: public Horizon::HSingletonBase<ImTaskManager>
	{
        struct TaskInternal : ImTaskInterface::Task
        {
            ImTaskInterface::TaskContext             Context;

            const ImTaskFlags        Flags;                              // is it visible in UI? (InsertImGuiWindow/InsertImGuiContent)

            Scope<ImTaskInterface>  TaskInstance;                               // the actual task

            TaskInternal(const std::string& name, ImTaskFlags flags, ImTaskInterface* task) : Task(name), Flags(flags), TaskInstance(task) { }

            TaskInternal(const TaskInternal&) = delete;
            TaskInternal& operator =(const TaskInternal&) = delete;
        };

		std::list<Ref<TaskInternal>>       m_currentUITasks;

	private:
        ImTaskManager() = default;
	public:
		~ImTaskManager() override = default;

		static void CreateManager();

        Ref<ImTaskInterface::Task> NewTask(ImTaskInterface* task, const std::string& taskName, ImTaskFlags flags = ImTaskFlags::None);

        void RenderUITask();

        virtual void FrameUpdate();

        virtual void FixedUpdate();
	};

    IMGUI_API Ref<ImTaskInterface::Task> NewUITask(ImTaskInterface* task, const std::string& taskName, ImTaskFlags flags = ImTaskFlags::None);

}
