# VGGalgameRuntime 模块架构与进展（摘要）

## SubsystemBus 重构（2026-05）

- **`IStoryScriptExecutor::Run(ISubsystemBus* bus, IGalGameContext* gameContext)`**：由 **`StoryScriptSystem::LoadStoryScript`** 以 **`m_GalGameEngine->GetSubsystemBus()`** 与 **`m_GalGameContext.get()`** 调用；Lua / Sequence 执行器已适配。
- **`StoryExecutionInstance`**：**`Tick(dt, bus)`** / **`Continue(bus)`** 转发至具体 **`IStoryScriptExecutor`**（Lua 侧忽略 bus；Sequence 内核由 **`SSExecutorSequence`** 传入 **`m_ExecutionContext.SubsystemBus`**）。
