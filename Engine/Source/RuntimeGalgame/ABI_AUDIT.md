# VisionGal Galgame — ABI / Contract 审计摘要（Phase 8H）

> 生成说明：随 Phase 8 里程碑维护；完整模块表见各 `MODULE_ARCHITECTURE_AND_PROGRESS.md`。

## 1. 跨 DLL 契约头（VGGalgameContract）

| 区域 | 规则 | 当前状态 |
|------|------|----------|
| Interface 反向 include Runtime 实现 | 禁止 | **IScriptRuntime** 仅依赖 **IStoryScriptExecutor** / **IStoryScriptSystem** / **Core** |
| 新增虚函数 | 需同步所有实现方 | **IScriptRuntime::TryCreateStoryExecution** — 实现类 **GalAssetTypeScriptRuntime** |
| `shared_ptr` 跨边界 | 禁止 | 仍使用 **Ref&lt;T&gt;** 聚合 |

## 2. 宿主 DLL（VGGalgame）

| 类型 | 职责 |
|------|------|
| **GalRuntimeCoordinator** | 生命周期编排；**不**持有业务子系统所有权（成员在 **GalGameEngine**） |
| **GalRuntimeScriptLoader** | 路径→执行器；无 **IGalGameEngine*** |
| **StoryScriptSystem** | **ISubsystemBus*** 注入；**IStoryExecutionInstance** 可为内核直出 |

## 3. 已知技术债（与规则对照）

| 条目 | 说明 |
|------|------|
| **Lua SSExecutor::HostEngine** | 仍持 **IGalGameEngine***；应迁 **IRuntimeExecutionServices** / **ISubsystemBus**（分 PR） |
| **ExecutionSignal 与 UI** | **GalYieldKind::Signal*** 已入契约；宿主调度器尚未订阅 UI 完成事件做统一 resume |

## 4. 建议命令

- `Engine/Scripts/check_vggalgame_core_includes.ps1` — 校验 Core 薄转发 include。
- `Engine/Source/RuntimeGalgame/merge_docs.py` — 合并各模块架构文档。
