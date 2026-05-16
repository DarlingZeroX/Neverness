#pragma once

#include "../imconfig.h"
#include <string>
#include <future>
#include <queue>
#include <shared_mutex>
#include <NNKernel/Interface/HCoreTypes.h>

namespace ImGuiEx {

    struct IMGUI_API ImTaskInterface
    {
        // used as a 'handle' for spawned tasks (visible from outside)
        struct Task
        {
            const std::string                Name;
            bool                        Result;

        protected:
            Task(const std::string& name) : Name(name), Result(false) { }
        };

        // used as a link between the vaBackgroundTaskManager and the running task (visible only from the task itself)
        struct TaskContext
        {
			size_t						Index = 0;
            std::atomic_bool            ForceStop = false;        // used for manager to signal to the task that it should drop all work and exit when possible safely; task should probably not modify it (maybe safe? never checked)
            std::atomic<float>          Progress = 0.0f;         // used to the task to indicate progress (mostly for UI purposes) - clamped [0, 1]
            std::atomic_bool            HideInUI = false;        // used for when ShowInUI flag set but one wants to hide/show it at runtime
            std::atomic_bool            IsFinished = false;                 // used to indicate the task function has finished (exited)
        };

        virtual ~ImTaskInterface() = default;

         virtual void RenderUI(TaskContext& context) = 0;

         virtual void FrameUpdate() {};
         virtual void FixedUpdate() {};
    };


}