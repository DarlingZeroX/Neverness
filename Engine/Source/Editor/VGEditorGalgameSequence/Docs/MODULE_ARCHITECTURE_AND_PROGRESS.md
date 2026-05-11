# VGEditorGalgameSequence 模块架构与开发进展

本文档描述 **Galgame 可视化序列脚本编辑器** 动态库目标（`VGEditorGalgameSequence`，CMake 中为 `SHARED`）的整体架构、各子系统职责、与引擎其他部分的协作方式，以及截至当前的实现进展与已知边界。

更细的「如何注册新序列组件 / Inspector」步骤见同目录下的 [SEQUENCE_EDITOR_REGISTRATION.md](SEQUENCE_EDITOR_REGISTRATION.md)。

---

## 1. 模块定位

`VGEditorGalgameSequence` 提供面向 **`.vgasset` 序列脚本资源** 的 ImGui 编辑体验：条目列表、组件调色板、属性检查器、撤销/重做、剪贴板、保存与「执行到某条目」的调试式步进。对外主入口为 `VisionGal::Editor::VGScriptSequenceEditor`（`Interface/SequenceEditor.h`），实现编辑器框架的 `IEditorTaskPanel`，并可由宿主通过 `RenderEmbeddedUI()` 嵌入同一套 UI。

**主要链接依赖**（见 `CMakeLists.txt`）：

- `VGEditorFramework`、`HNGEditorCore`：编辑器任务面板与基础设施。
- `VGGalgame`：Galgame 侧资源与引擎集成。
- `VGCore`、`HCorePlatform`、`HFileSystem`：核心服务、原生保存对话框、路径与 VFS。

序列数据本身来自 **`VGGalgameScriptSequence`**（`VGSSequenceDataContainer`、`IVGSSequenceComponent` 等）；本模块不重复定义运行时组件类型，而是通过 `IVGSSequenceComponentManager::EnumerateRegisteredTypeNameIDs` 与运行时注册表对齐。

---

## 2. 源码目录结构

| 路径 | 职责 |
|------|------|
| `Interface/` | 对外 API：`SequenceEditor.h`（`VGScriptSequenceEditor`）、`VGEGSExport.h`。 |
| `Include/Document/` | `SequenceDocument`：资源路径、脏标记、对 `VGSSequenceDataContainer` 的封装与读写。 |
| `Include/Core/` | `SequenceEditorContext`、选择、撤销栈、剪贴板；`SequenceEditorEvents.h` 转发至 `Events/SequenceEditorEvent.h`。 |
| `Include/Events/` | `SequenceEditorEvent`、`SequenceEditorEventType`、`SequenceEditorEventBus`（编辑器主线程发布/订阅）。 |
| `Include/Services/` | `SequenceEditorServiceLocator`、`SequenceValidationCacheService`、`SequenceSearchIndexService`。 |
| `Include/Async/` | `SequenceAsyncTaskService`、`SequenceBackgroundValidationTask`（后台校验任务封装）。 |
| `Include/AssetMonitoring/` | 资源依赖/热更占位（`SequenceAssetMonitoringFwd.h` 等，后续迭代）。 |
| `Include/Commands/` | `ISequenceEditorCommand` 及增删改、移动、粘贴、属性编辑、复合命令。 |
| `Include/ComponentRegistry/` | 组件元数据、注册表、Bootstrap 声明。 |
| `Include/Inspector/` | `ISequenceInspector`、内置实现工厂、注册表。 |
| `Include/Runtime/` | `SequenceExecutionController`、`SequenceRuntimeSession`、`SequenceRuntimeSnapshot`、`SequenceRuntimeObserver`、`SequenceRuntimeOverlayState`。 |
| `Include/ViewModels/` | `SequenceDocumentViewModel`、`SequenceEntryViewModel`、`SequenceSearchViewModel`（展示层只读模型）。 |
| `Include/Validation/` | `SequenceValidationRegistry`、`ISequenceValidator`、`SequenceValidationIssue` 及 `Builtin/` 内置规则。 |
| `Include/Timeline/` | 线性时间轴 v1：`SequenceTimelineLayout`、`SequenceTimelineController`、`SequenceTimelineWidget`。 |
| `Include/Widgets/` | 工具栏、条目列表、调色板、搜索、Inspector、校验面板、大纲、状态栏、时间轴等 ImGui 控件。 |
| `Source/` | 与上述头文件对应的 `.cpp` 实现；含 `Events/`、`Services/`、`Async/`、`AssetMonitoring/`；`VGSequenceEditor.cpp` 为宿主级编排逻辑。 |

---

## 3. 总体架构

核心思想：**单一宿主类持有文档与工具对象，通过 `SequenceEditorContext` 把只读/可写依赖注入各 Widget**，文档变更优先走 **命令 + `SequenceUndoStack`**，以便统一撤销/重做。

```mermaid
flowchart TB
  subgraph Host["VGScriptSequenceEditor 宿主"]
    Doc[SequenceDocument + GenerationId]
    RegC[SequenceComponentRegistry]
    RegI[SequenceInspectorRegistry]
    RegV[SequenceValidationRegistry]
    ValCache[SequenceValidationCacheService]
    SearchIdx[SequenceSearchIndexService]
    Session[SequenceRuntimeSession]
    Exec[SequenceExecutionController]
    Obs[SequenceRuntimeObserver]
    Async[SequenceAsyncTaskService]
    Bus[SequenceEditorEventBus]
    Loc[SequenceEditorServiceLocator]
    Undo[SequenceUndoStack]
    Sel[SequenceSelectionModel]
    Clip[SequenceClipboard]
    DocVM[SequenceDocumentViewModel]
    Ctx[SequenceEditorContext]
    Tick[TickEditorPresentation]
    Doc --> Tick
    RegC --> Tick
    RegV --> ValCache
    ValCache --> Tick
    SearchIdx --> Tick
    Tick --> DocVM
    DocVM --> Ctx
    Session --> Exec
    Session --> Obs
    Bus --> Sel
    Bus --> Ctx
    Loc --> Ctx
    Doc --> Ctx
    Exec --> Ctx
    Undo --> Ctx
    Sel --> Ctx
    Clip --> Ctx
    RegI --> Ctx
    ValCache --> Ctx
    Async --> Ctx
  end

  subgraph UI["ImGui Widgets"]
    TB[SequenceToolbarWidget]
    Pal[SequenceComponentPaletteWidget]
    Search[SequenceSearchWidget]
    List[SequenceEntryListWidget]
    Insp[SequenceInspectorWidget]
  end

  Ctx --> TB
  Ctx --> Search
  Ctx --> List
  Ctx --> Insp
  Pal -->|OnComponentChosen| Cmd[AddSequenceEntryCommand via ExecuteCommand]

  subgraph Runtime["VGGalgameScriptSequence / Galgame 引擎"]
    Asset[SequenceScriptAsset Loader/Writer]
    Mgr[IVGSSequenceComponentManager]
  end

  Doc <--> Asset
  RegC --> Mgr
  Exec -->|LoadStoryScript + Step| Engine[GameEngineCore / StoryScriptSystem]
```

---

## 4. 子系统说明

### 4.1 宿主：`VGScriptSequenceEditor`

- 构造路径分支：无参构造创建空文档并 `FillDefaultDemoEntries()`；带路径构造则 `LoadFromAssetPath`，并在进入场景播放模式时通过 `EngineEventBus` 自动 `SaveAsset()`。
- `InitializeChrome()`：Bootstrap 组件 / 校验 / 检查器注册表；装配 `SequenceEditorServiceLocator`（校验缓存、搜索索引、运行时会话、异步任务）；`SequenceRuntimeSession::Bind` 控制器与 Observer 及事件总线；`SequenceSelectionModel::SetEventBus`；将 `SequenceEditorContext` 的 `onDocumentMutationAccumulate` / `requestPresentationRefresh` 指向宿主，用于合并脏摘要与置位「需要编排刷新」；首帧 `TickEditorPresentation()`；刷新调色板并订阅 `OnComponentChosen` → `ExecuteCommand(AddSequenceEntryCommand)`。
- **`TickEditorPresentation()`（取代每帧多次 `SyncContext`）**：在 `m_needsPresentationTick` 或首帧时，根据合并后的 `SequenceDocumentMutationSummary` 对 `SequenceDocumentViewModel` 做 **全量 `Rebuild`**（结构变更）或 **`RebuildEntriesAtIndices`**（仅内容/局部）；`SequenceSearchIndexService::RebuildFromViewStorage`；`SequenceValidationCacheService::ApplyIfStale` → `ApplyValidationIssues`；可选发布 `ValidationUpdated`；`ApplyRuntimeOverlay`；`ApplySearchViewModelWithIndex`（文本维度 + 索引候选 + 原有维度谓词）。
- **`FillContextPointers()`**：仅把指针写入 `SequenceEditorContext`（`document`、`documentViewModel`、`validationRegistry`、`validationCache`、`runtimeOverlay`、`services`、`eventBus`、`execution` 等），**不**再触发全链路重建。
- `RenderEditorBody()`：帧首 `SequenceAsyncTaskService::PumpCompleted` → `TickEditorPresentation`；渲染各 Dock 窗口与序列区；快捷键；帧尾再次 `PumpCompleted` + `TickEditorPresentation`，以吸收同帧内搜索/控件触发的刷新请求，并将多次脏标记合并为有限次编排。
- `ExecuteTo(index)`：保存资源后委托 **`SequenceRuntimeSession::RequestRunTo`**（内部调用 `SequenceExecutionController::ExecuteTo` + `SequenceRuntimeObserver::NotifyExecuteCompleted`，并在总线发布 `RuntimeStateChanged`）；结束后置位 `m_needsPresentationTick` 以刷新运行时高亮。
- `OpenAsset`：成功后 `validationCache.InvalidateAll` 并 `NotifyDocumentChanged`（结构级摘要），驱动全量重建与校验。

### 4.2 文档层：`SequenceDocument`

- 内部持有 `Ref<VGSSequenceDataContainer>` 与 `m_assetPath`、`m_dirty`。
- **编辑代次**：`GetGenerationId()` / `BumpEditGeneration()`（由 `SequenceEditorContext` 在成功 `ExecuteCommand` / `UndoDocument` / `RedoDocument` 后调用，用于校验缓存等与文档版本对齐）。
- **结构修订**：`GetStructureRevision()` / `BumpStructureRevision()`（加载、重置、演示填充、清空为新文档等路径 bump，与代次一并递增），便于后续区分「仅内容变更」与「条目集合/顺序变化」。
- 加载/保存通过 `GalGame::SequenceScriptAssetLoader` / `SequenceScriptAssetWriter` 与 `SequenceScriptAsset` 交互。
- 仍提供 `GetSequence()` 及若干直接修改序列的 API，头文件中标注为 **LEGACY**（尚未全部迁移到命令的 UI 路径仍可能使用）。
- 辅助：`InsertEntryAt`、`SetSequenceEntries`、重排接口等供命令与剪贴板使用。

### 4.3 编辑上下文：`SequenceEditorContext`

- **`ExecuteCommand`**：`SequenceUndoStack::ExecuteCommand` → `document->BumpEditGeneration()` → 从 `PeekUndoTop()->DescribeExecutedMutation()` 取 `SequenceDocumentMutationSummary` → `NotifyDocumentChanged`（写入校验缓存脏队列、累积宿主脏摘要、总线 `DocumentChanged`、请求 `TickEditorPresentation`）。
- **`UndoDocument` / `RedoDocument`**：供工具栏等调用；在文档 bump 代次后发布 **结构级** `NotifyDocumentChanged`（保守全量路径）。
- **`NotifyDocumentChanged` / `RequestPresentationRefresh`**：Widget 或宿主在不经由命令栈的场景（如工具栏「新建」清空文档）可显式通知；搜索控件在过滤变化时发布 `SearchFilterChanged`（经总线）并 `RequestPresentationRefresh`。
- 指针域：`document`、`documentViewModel`、`validationRegistry`、**`validationCache`**、**`eventBus`**、**`services`**（`SequenceEditorServiceLocator`）、`runtimeOverlay`、`searchFilter`、`executeToEntry` / `executeToUserData`、`lastExecutionSnapshot`；**`onDocumentMutationAccumulate` / `requestPresentationRefresh`** 为宿主提供的 C 风格回调，用于合并脏标记而不在 Widget 内直接重建 ViewModel。
- 列表 / 时间轴 / 大纲仍只读 `GetVisibleEntries()`；业务修改仍应通过 `ExecuteCommand`。

### 4.4 命令与撤销：`ISequenceEditorCommand` / `SequenceUndoStack`

| 命令 | 作用 |
|------|------|
| `AddSequenceEntryCommand` | 按 `TypeNameID` 追加条目。 |
| `RemoveSequenceEntryCommand` | 按索引集合删除。 |
| `MoveSequenceEntryCommand` | 拖拽重排。 |
| `PasteSequenceEntriesCommand` | 在指定位置插入克隆条目列表。 |
| `EditSequencePropertyCommand` | 配合 `SequenceEditFieldId` 做可撤销字段编辑（当前主要用于普通对话 Inspector）。 |
| `CompoundSequenceCommand` | 组合多条命令为一次撤销单元。 |

标准三操作：`Execute` / `Undo` / `Redo` 均接收 `SequenceDocument&`。

- **`DescribeExecutedMutation()`**：返回 `SequenceDocumentMutationSummary`（`TouchedIndices` + `StructuralChange`），供宿主在 `ExecuteCommand` 成功后做增量 ViewModel 与增量校验；默认基类实现为「结构级」全量降级。具体命令（增删、移动、粘贴、属性编辑、复合命令）均已覆盖。
- **`SequenceUndoStack::PeekUndoTop()`**：供上下文在压栈后读取刚执行命令的变更摘要。

### 4.5 选择与剪贴板

- `SequenceSelectionModel`：单选、Ctrl 多选、`ClampToSize`；变更时若已 `SetEventBus`，则发布 **`SelectionChanged`**。
- `SequenceClipboard`：深拷贝选中条目；`TryPaste` 通过 `PasteSequenceEntriesCommand` 插入；`CutSelection` 注释中说明完整「剪切」语义可能需要与复合命令组合。

### 4.6 组件注册：`SequenceComponentRegistry` 与 Bootstrap

- `BootstrapSequenceComponentRegistry` 遍历运行时注册的所有 `TypeNameID`，填充 `SequenceComponentMetadata`（展示名、图标、分类、优先级）。
- 内置三种类型（普通对话、切换立绘、切换背景）在 `SequenceEditorRegistriesBootstrap.cpp` 的 `FillPresentationForTypeNameID` 中写中文名与 FontAwesome 图标；其余类型使用默认立方体图标与分类「序列组件」。
- `BuildPaletteCategories()` 为调色板提供分类列数据。

### 4.7 检查器：`SequenceInspectorRegistry` / `ISequenceInspector`

- `ISequenceInspector` 预留 `OnInspectorGUI`、`OnHeaderGUI`、`OnTimelineGUI`、`OnContextMenu` 等钩子，便于未来时间轴或图形式编辑。
- `MakeSequenceInspectorForMetadata`（`BuiltinSequenceInspectors.cpp`）按类型分派：
  - **普通对话**：带 staging 字符串，失焦后通过 `EditSequencePropertyCommand` 写入（支持撤销）。
  - **切换立绘 / 切换背景**：直接 ImGui 绑定组件字段；背景支持从内容浏览器拖入纹理路径。
  - **其他类型**：`FallbackSequenceInspector`（空面板，但视为已注册）。

### 4.8 运行时步进：`SequenceExecutionController` 与 `SequenceRuntimeSession`

- **`SequenceExecutionController`**：若当前非播放模式则 `EnterPlayMode`；通过 `GalGame::GameEngineCore` 取得故事脚本系统，`LoadStoryScript(assetPath)` 后循环 `Continue`/`Tick`，直到 `SSSequenceRuntimeDebugInfo::CurrentIndex` 达到目标索引或触发步数上限 / 停滞检测。
- 结果写入 `SequenceRuntimeSnapshot`（当前索引、是否到达目标、错误字符串等），**不保存引擎指针**。
- **`SequenceRuntimeSession`**：编辑器侧薄封装；`RequestRunTo` 调用控制器、`NotifyExecuteCompleted` 更新 overlay，并在 **`SequenceEditorEventBus` 上发布 `RuntimeStateChanged`**（载荷含是否成功、是否到达目标、高亮索引等）。状态枚举 `Idle` / `Running` 等为后续 Step、断点、时间轴跟随预留。

### 4.9 展示层 ViewModel、搜索索引与校验

- **ViewModel**：`Rebuild` 全量同步 `m_storage` 与可见行；**`RebuildEntriesAtIndices`** 在存储长度与文档一致时仅刷新指定行，否则回退全量。**`GetEntryStorage()`** 供搜索索引重建。
- **搜索**：**`ApplySearchViewModelWithIndex`** 在启用文本维度且过滤串非空时，先用 **`SequenceSearchIndexService::QueryTextIndices`** 得到候选行索引，再对候选行执行 `SequenceSearchViewModel::RowPassesFilters`（与旧版 `ApplySearchViewModel` 行为对齐，但减少全表字符串扫描）。
- **校验展示**：**`ApplyValidationIssues`** 接受 issue 列表（来自缓存或全量）；**`ApplyValidation(registry, document)`** 仍直接 `RunAll`，供测试与降级路径。
- **`SequenceValidationCacheService`**：合并多次 `NotifyDocumentChanged`；`ApplyIfStale(document, registry, generationId)` 在文档代次未变且非 stale 时 **跳过**；结构变更或空 `touched` 时 **`RunAll`**，否则 **`RunForEntries`**（见下）。
- **`SequenceValidationRegistry`**：`RunAll` 保留；新增 **`RunForEntries`**，对每个校验器调用 **`ISequenceValidator::ValidateEntries`**（默认实现回退为全量 `Validate`）。内置 **`EmptyDialogueValidator`**、**`MissingResourcePathValidator`** 已实现按行增量扫描。

### 4.10 事件总线、服务定位与异步任务

- **`SequenceEditorEventBus`**（每编辑器实例一个）：`Subscribe(SequenceEditorEventType, Listener)` / `Publish` / `Clear`；约定仅在 **编辑器主线程 / ImGui 帧内** 使用，首版无锁。
- **`SequenceEditorEventType`**：`DocumentChanged`（与历史名 `DocumentMutated` 同值）、`SelectionChanged`、`ValidationUpdated`、`RuntimeStateChanged`、`SearchFilterChanged`；载荷见 `Include/Events/SequenceEditorEvent.h`（文档变更摘要、运行时摘要等）。
- **`SequenceEditorServiceLocator`**：向 Context 暴露 `validationCache`、`searchIndex`、`runtimeSession`、`asyncTasks` 指针，避免 Widget 自行 `new` 服务。
- **`SequenceAsyncTaskService`**：`EnqueueValidation` 在 **detach 的 `std::thread`** 上执行生产函数，完成项入队；宿主每帧 **`PumpCompleted`** 在主线程执行合并回调（禁止在 worker 上碰 ImGui）。**`SequenceBackgroundValidationTask`** 为对上述 API 的薄封装。当前宿主 **尚未** 在常规编辑路径自动排队后台全量校验，基础设施供后续大文档或 debounce 策略接入。
- **`Include/AssetMonitoring/`**：占位头文件与 `Source/AssetMonitoring/SequenceAssetMonitoringStub.cpp`（满足 CMake 目录纳入）；依赖图 / 热更监听待后续与 `DocumentChanged` 联动。

### 4.11 展示层 Widget（简要）

| Widget | 行为摘要 |
|--------|----------|
| `SequenceToolbarWidget` | File（新建时 `NotifyDocumentChanged` 结构级摘要）、Edit（**`UndoDocument` / `RedoDocument`**）、Play（对唯一选中项 Execute To）。 |
| `SequenceStatusBarWidget` | 资源路径、脏标记、校验问题计数、`runtimeOverlay` 中的运行错误。 |
| `SequenceComponentPaletteWidget` | 消费注册表分类，展示可添加组件。 |
| `SequenceSearchWidget` | 文本过滤 + 维度开关；过滤或维度变化时经总线发布 **`SearchFilterChanged`**，并 **`RequestPresentationRefresh`**（不直接重建 ViewModel）。 |
| `SequenceEntryListWidget` | 仅遍历 `GetVisibleEntries()`；行内 Exec、Ctrl 多选、拖拽 `MoveSequenceEntryCommand`、关闭折叠触发删除；大行数时 `ImGuiListClipper` 紧凑行虚拟化。 |
| `SequenceTimelineWidget` | 线性行条、选中与拖拽重排（同命令）；无曲线与多轨道。 |
| `SequenceOutlinerWidget` | 按 `Category` 分组展示当前可见行。 |
| `SequenceValidationWidget` | 列出校验问题，点击跳转选中条目索引。 |
| `SequenceInspectorWidget` | 单选时按 `TypeNameID` 从注册表绘制 Inspector；多选提示不显示属性。 |

### 4.12 集成入口

- **任务面板**：`ModuleEditorGalgame.cpp` 中 `NewTask(new VGScriptSequenceEditor(path), ...)`。
- **嵌入 Visual GalGame Editor**：`VisualGalgame.h` 内嵌 `VGScriptSequenceEditor`，调用 `RenderEmbeddedUI()`。

---

## 5. 数据流概要

1. 宿主构造 `SequenceDocument`，Bootstrap 组件 / 检查器 / 校验注册表，装配总线、服务定位、运行时会话与 Context 回调；**`TickEditorPresentation()`** 按合并后的 `SequenceDocumentMutationSummary` 更新 ViewModel、搜索索引、校验缓存、overlay 与可见行，**`FillContextPointers()`** 仅写 Context 指针。
2. Widget 通过 context **只读**文档与 ViewModel；业务修改应调用 **`context.ExecuteCommand(...)`**（内部 bump 文档代次、`NotifyDocumentChanged`、请求编排刷新）。工具栏撤销/重做走 **`UndoDocument` / `RedoDocument`**。
3. **脏合并**：一帧内多次 `RequestPresentationRefresh` 折叠为有限次 `TickEditorPresentation`（帧首 + 帧尾各一次，吸收搜索等同帧变更）。
4. 资源持久化由 `SequenceDocument` 与 `VGGalgameScriptSequence` 资源管线完成。
5. 「执行到某行」路径：保存 → **`SequenceRuntimeSession::RequestRunTo`** → `SequenceExecutionController` → `SequenceRuntimeSnapshot` → `SequenceRuntimeObserver` → overlay；总线 **`RuntimeStateChanged`**；宿主置位下一帧 presentation 以刷新行高亮与状态栏。
6. **可选异步**：`SequenceAsyncTaskService` 在 worker 上计算 issue 列表，主线程 `PumpCompleted` 中可调用 **`SequenceValidationCacheService::ReplaceIssues`** 并发布 `ValidationUpdated`（具体触发策略待产品化）。

---

## 6. 当前开发进展

### 6.1 已具备的能力

- 序列文档的加载、保存、另存为、未命名重置与脏标记跟踪。
- 与运行时类型列表对齐的组件调色板；内置三类组件的展示信息与专用 Inspector；其余类型有 Fallback。
- 条目列表与时间轴：基于 ViewModel 可见行、搜索维度、校验高亮与运行时高亮；选择、逐行执行、拖拽排序、删除；调色板添加条目。
- 撤销栈与命令对象覆盖增删、移动、粘贴、对话字段编辑；工具栏与 Ctrl+C/X/V 剪贴板流程。
- 工具栏「Execute To 选中项」与列表行「Exec」：`ExecuteTo` 前自动保存，快照反馈错误信息。
- 关闭带脏文档时的确认弹窗（任务面板模式）。
- 单元测试 `VGEditorGalgameSequenceTest`：文档 SaveAs/Reset、撤销添加、剪贴板复制粘贴、粘贴命令撤销；**事件总线投递**、**ExecuteCommand 后代次递增**、**校验缓存稳定代次跳过与增量 `ValidateEntries`**、**异步任务 Pump 合并回调**（见同目录测试源文件）。
- ViewModel / 校验：`Rebuild` 行数与文档一致、搜索过滤可清空可见行、内置校验器在演示文档上产生问题的 gtest。

### 6.2 部分完成或过渡状态

- `SequenceDocument::GetSequence()` 等 **LEGACY** 直接访问仍存在，部分 Inspector（立绘/背景）仍直接改组件指针字段，**未全部**走 `EditSequencePropertyCommand`，与「全字段可撤销」目标不一致。
- **`SequenceAsyncTaskService`** 已就绪，但编辑器 **尚未** 在默认路径自动调度后台全量校验（需 debounce、与缓存合并策略及 UI 提示一并设计）。
- **`SequenceEditorSettings`** 仍为独立设置占位，与总线无直接耦合。
- `ISequenceInspector` 的 Header / Timeline / ContextMenu 钩子多数未实现具体 UI。
- **`AssetMonitoring`** 仅占位目录，未实现依赖图与热更。

### 6.3 第三阶段（Presentation Layer）已落地要点

- **ViewModel**：`SequenceDocumentViewModel` + `SequenceEntryViewModel`；列表 / 时间轴 / 大纲只消费 `GetVisibleEntries()`；校验与运行时高亮仍通过 ViewModel 字段体现。
- **校验**：`SequenceValidationRegistry` + Builtin 规则；`SequenceValidationWidget` 与状态栏展示问题数量。
- **运行时**：`SequenceRuntimeObserver` + `SequenceRuntimeOverlayState`；列表高亮读 ViewModel，Widget 不轮询引擎。
- **搜索**：`SequenceSearchViewModel` 维度与可见行过滤；第四阶段起生产路径结合 **搜索索引**（见 6.4）。
- **其他 UI**：`SequenceStatusBarWidget`；大行数下列表 `ImGuiListClipper` 虚拟化紧凑行。

### 6.4 第四阶段（增量编辑器基础设施）已落地要点

- **事件**：`SequenceEditorEventBus` + `SequenceEditorEvent` / `SequenceEditorEventType`；文档变更、选择、校验刷新、运行时、搜索过滤均有发布点（见 4.10、4.3、4.5、4.1）。
- **文档代次**：`SequenceDocument::GetGenerationId` / `GetStructureRevision` 与 bump 规则；`SequenceValidationCacheService::ApplyIfStale` 与代次对齐以避免无变更重复 `RunAll`。
- **帧编排**：`TickEditorPresentation` + `FillContextPointers` 取代「每帧多次全链路 `SyncContext`」；脏摘要合并与帧尾二次 tick 收敛同帧成本。
- **增量 ViewModel**：`RebuildEntriesAtIndices` + 结构变更时全量 `Rebuild`；`GetEntryStorage` 驱动索引重建。
- **校验缓存与增量 API**：`RunForEntries` + `ISequenceValidator::ValidateEntries`（默认回退全量）；内置校验器已实现按行扫描。
- **搜索索引**：`SequenceSearchIndexService` + `ApplySearchViewModelWithIndex`。
- **运行时会话**：`SequenceRuntimeSession` 包装 `ExecuteTo` 与 overlay 更新并发布 `RuntimeStateChanged`。
- **构建**：`CMakeLists.txt` 已 GLOB `Events`、`Services`、`Async`、`AssetMonitoring` 子目录。

### 6.5 建议后续工作（非承诺路线图，仅反映代码缺口）

- 将立绘、背景等 Inspector 字段逐步改为命令式编辑，并补齐多选/批量操作策略。
- 在编辑路径接入 **后台校验**（debounce、`ReplaceIssues`、与增量缓存一致性、进行中状态 UI）。
- 深化 **AssetMonitoring**：资源路径变更 → 标记受影响 entry 校验脏。
- 扩展测试覆盖：移动命令、复合命令、执行控制器错误分支等（需可 mock 引擎时更易维护）。
- 多面板（如未来节点图）订阅总线事件，进一步减少宿主硬编码依赖。

---

## 7. 相关文档与代码入口

| 主题 | 位置 |
|------|------|
| 注册新组件与 Inspector | [SEQUENCE_EDITOR_REGISTRATION.md](SEQUENCE_EDITOR_REGISTRATION.md) |
| 宿主编排与 UI 布局 | `Source/VGSequenceEditor.cpp`、`Interface/SequenceEditor.h` |
| 事件与服务 | `Include/Events/`、`Include/Services/`、`Include/Async/` |
| 运行时组件定义 | `Engine/Source/Runtime/VGGalgameScriptSequence/`（及 `IVGSSequenceComponentManager` 注册） |

---

## 8. 修订记录

| 日期 | 说明 |
|------|------|
| 2026-05-11 | 初版：完整架构说明与进展对齐当前代码树。 |
| 2026-05-11 | 补充第三阶段 Presentation：ViewModel、校验、运行时 Overlay、时间轴与相关测试说明。 |
| 2026-05-12 | 第四阶段：事件总线、帧编排 `TickEditorPresentation`、文档代次、校验缓存与 `ValidateEntries`、搜索索引、运行时会话、异步任务占位；更新目录表、数据流与测试说明。 |
