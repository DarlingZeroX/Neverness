# Galgame 序列编辑器文档

本文档包含两部分：**（A）** 运行时与编辑器的组件注册流程；**（B）** 第三阶段「展示层 / Presentation Layer」架构与实现计划。二者互补：注册解决「类型如何进编辑器」，第三阶段解决「UI 如何稳定地消费文档与运行时」。

---

## A. 注册新的 Galgame 序列组件 (运行时 + 编辑器)

以下章节说明了如何使新的 `IVGSSequenceComponent` 类型在 `VGEditorGalgameSequence` 中变为可编辑状态，而无需手动维护重复的列表。

## 1. 运行时类型与工厂 (VGGalgameSequenceRuntime)
在脚本序列模块中定义组件（通常使用 TVGSSequenceComponent<YourType>）。

在 IVGSSequenceComponentManager 中进行注册（参考 IVGSSequenceComponent.cpp 中的 EmplaceComponentType<YourType>()）。
IVGSSequenceComponentManager::EnumerateRegisteredTypeNameIDs 是“存在哪些类型”的唯一事实来源。


## 2. 编辑器调色板与检查器引导程序 (Bootstrap)
BootstrapSequenceComponentRegistry 会遍历由 EnumerateRegisteredTypeNameIDs 返回的所有 ID，并为每个 ID 构建 SequenceComponentMetadata：

- 内置三种组件（普通对话、切换立绘、切换背景）的展示名、FontAwesome 图标与分类在 `Engine/Source/Editor/VGEditorGalgameSequence/Source/ComponentRegistry/SequenceEditorRegistriesBootstrap.cpp` 的 `FillPresentationForTypeNameID` 中配置。
- 其余类型默认使用 `DisplayName = TypeNameID`、`Category = 序列组件`、`Icon = 立方体图标`。

随后，`BootstrapSequenceInspectorRegistry` 对每个元数据条目调用 `MakeSequenceInspectorForMetadata`（定义于 `Engine/Source/Editor/VGEditorGalgameSequence/Include/Inspector/BuiltinSequenceInspectors.h`），向 `SequenceInspectorRegistry` 注册对应的 `ISequenceInspector` 实现。

## 3. 扩展内置展示与检查器 UI
为新类型提供与内置三项类似的固定中文名 / 图标 / 分类时：在 `FillPresentationForTypeNameID`（同上 bootstrap 源文件）中按 `TypeNameID` 增加分支即可。

为某类型提供属性面板时：在 `Engine/Source/Editor/VGEditorGalgameSequence/Source/Inspector/BuiltinSequenceInspectors.cpp` 中实现新的 `ISequenceInspector` 子类，并在 `MakeSequenceInspectorForMetadata` 中按 `meta.TypeNameID` 分支返回 `std::make_unique<YourInspector>(...)`。未单独实现的类型会使用 `FallbackSequenceInspector`（面板为空，但仍视为已注册）。

## 4. 支持撤销的检查器字段 (Undo-aware)
对于高频修改的字段：

扩展 `SequenceEditFieldId` / `EditSequencePropertyCommand`，或使用 **`SetSequenceEntryBoolPropertyCommand`**（参考 `BuiltinSequenceInspectors.cpp` 中内置三项的实现）。

或者，为该类型添加独立的 `ISequenceInspector` 源文件并在工厂函数中挂接，以保持单文件职责清晰。

---

## B. 第三阶段：Presentation Layer（展示层）架构与实现计划

> **文档目的**：在第二阶段（Document / Command / Selection / Clipboard / Registry / Widget / Context）已完成的前提下，定义第三阶段的**目标架构、编码铁律、目录规划、分步迁移与验收标准**。实现时以本章节为单一事实来源（SSOT），避免 Widget 与 `VGSSequenceDataContainer` 继续紧耦合。

### B.0 第二阶段与第三阶段的边界

| 维度 | 第二阶段（已完成） | 第三阶段（本计划） |
|------|-------------------|---------------------|
| 数据流 | Widget 可直接读 `SequenceDocument` / 序列内部结构 | Widget **优先**只读 ViewModel；禁止遍历原始 `SequenceData` 作为列表数据源 |
| 编辑 | Command + Undo/Redo | **不变**；Timeline 等只发命令，不直接改 Document |
| 校验 | Inspector 内可散落临时检查 | **统一** `ValidationRegistry` → `ValidationIssue` |
| 运行时 UI | Widget 可能直接拉取或隐式依赖 Runtime | **Observer** 推送 → `OverlayState` → Widget |

> **说明**：第三阶段**不是** Graph / Node / Pin / ECS Timeline；当前 Runtime 仍是**线性序列**，过早 Graph 化会强制引入 Node Runtime、边执行、Jump 解析器等，复杂度与第二阶段成果不对齐。

---

### B.1 核心目标与数据流

**目标数据流**：

```text
Widget → ViewModel → SequenceDocument → SequenceData（运行时事实）
         ↑              ↑
    展示缓存/过滤    唯一可变源 + Command
         ↑
  Timeline / Search / Validation / RuntimeOverlay 均在此层组合
```

**非目标数据流（第三阶段起逐步废止）**：

```text
Widget ──直接遍历──► document->GetEntryAt(i) 循环驱动列表/时间轴  // 仍禁止；应使用 ViewModel 可见行
```

**ViewModel 层职责（集中说明，便于代码评审对照）**：

- **UI 缓存**：避免每帧从 Document 深扫；`Rebuild` 与增量更新策略明确。
- **搜索 / 过滤 / 排序**：维度见 **B.8**（`SequenceSearchViewModel`）；结果反映为「可见条目」与索引，不污染 `SequenceData`。
- **Runtime 状态**：只消费 `RuntimeObserver` 推送的摘要状态，不轮询。
- **Validation 状态**：只消费 `ValidationRegistry` 产出的 `ValidationIssue` 聚合（按 Entry 索引映射）。
- **可见性与展示字段**：`DisplayName` / `Subtitle` / `Category` / `Icon` 等与 `SequenceComponentMetadata`、本地化策略对齐；Widget 不直接拼业务字符串。

---

### B.2 编码铁律（四条 + 落地要点）

#### 铁律 1：Widget 禁止直接遍历 `SequenceData` 作为 UI 数据源

- **禁止**：绕过 ViewModel，以 `document->GetEntryCount()` + `GetEntryAt` 手写循环驱动列表、时间轴、Outliner（与遍历 `m_Sequence` 等价，仍属紧耦合）。
- **必须**：`viewModel->GetVisibleEntries()`（或等价 API）返回 `SequenceEntryViewModel` 的**只读视图**。
- **落地要点**：Document 变更（Command 提交、外部加载）后，由 **明确生命周期**（如 `Rebuild`、`OnDocumentChanged`）刷新 ViewModel；避免 Widget 内「顺手再扫一遍」。

#### 铁律 2：Timeline 不允许直接修改 Document

- Timeline 仅负责：**行布局、轨道占位（v1 可为单轨语义）、选择叠加层、Runtime 叠加层、时间与滚动映射、Hover**。
- **编辑**（重排、删除、属性修改）：仍走 **Command System**；Timeline 只生成 `MoveSequenceEntryCommand` 等已有或新增命令。
- **落地要点**：引入 `SequenceTimelineController` 作为「视口与交互状态机」，与 `SequenceDocument` 通过 **Context + CommandExecutor** 交互，不持有 Document 的可变裸指针进行写入。

#### 铁律 3：Validation 必须统一化

- **禁止**：Inspector 内「临时弹个红字」作为唯一错误来源。
- **必须**：`SequenceValidationRegistry` 注册若干 `ISequenceValidator`，对 Document（或快照）生成 `SequenceValidationIssue` 列表；Inspector 仅**展示**已注册问题（可选高亮字段）。
- **落地要点**：严重级别 `SequenceValidationSeverity` 与 UI 颜色/图标映射在**一处**配置，避免各 Widget 各写一套。

#### 铁律 4：Runtime Overlay 必须 Observer 化

- **禁止**：Widget `Update` 中主动 Pull `SequenceExecutionController` / Runtime 细节。
- **必须**：`SequenceRuntimeObserver` 订阅执行事件或轮询（**仅限 Observer 内部**）→ 写入 `SequenceRuntimeOverlayState` → Widget 只读 Overlay。
- **落地要点**：`SequenceRuntimePreviewSession` 与编辑器「试播」生命周期绑定；暂停/错误/当前条目索引等均为 Overlay 状态字段，便于单元测试与多视图同步。

---

### B.3 目标目录结构（Include / Source 对齐）

以下路径相对于 `Engine/Source/Editor/VGEditorGalgameSequence/`。

```text
Include/
├── ViewModels/
│   ├── SequenceDocumentViewModel.h      // 文档级：Rebuild、过滤、排序、聚合状态
│   ├── SequenceEntryViewModel.h         // 单行展示模型（禁止 Widget 直读 Component）
│   ├── SequenceTimelineViewModel.h      // 时间轴专用：行几何、可见窗口、与 Document VM 同步
│   ├── SequenceSearchViewModel.h        // 查询模型：类型/分类/校验/运行时维度
│   └── SequenceValidationViewModel.h    // 可选：将 Issue 列表按 Entry/严重级别建索引供 UI 快速查询
├── Timeline/
│   ├── SequenceTimelineController.h
│   ├── SequenceTimelineTrack.h          // v1：可为单轨或「逻辑轨」占位，禁止多轨剧情剪辑语义
│   ├── SequenceTimelineRow.h
│   ├── SequenceTimelineLayout.h
│   ├── SequenceTimelineViewport.h
│   └── SequenceTimelineSelection.h      // 与 SelectionModel 的关系见 B.6
├── Validation/
│   ├── SequenceValidationIssue.h
│   ├── ISequenceValidator.h
│   ├── SequenceValidationRegistry.h
│   └── Builtin/                         // 内置校验器：缺资源、非法跳转等
├── Runtime/                             // 与已有 Runtime 头文件共存时注意命名前缀，避免 ODR/语义混淆
│   ├── SequenceRuntimeObserver.h
│   ├── SequenceRuntimeOverlayState.h
│   ├── SequenceRuntimePreviewSession.h
│   └── SequenceRuntimeSelectionSync.h   // 编辑器选择 ↔ 运行时高亮（可选 v1.5）
└── Widgets/                            // 在现有 Widgets 下新增
    ├── SequenceTimelineWidget.h
    ├── SequenceValidationWidget.h
    ├── SequenceRuntimeWidget.h
    ├── SequenceOutlinerWidget.h
    └── SequenceStatusBarWidget.h

Source/
├── ViewModels/
├── Timeline/
├── Validation/
└── Runtime/                             // Observer 等实现；与 Include/Runtime 一一对应
```

> **说明**：`CMakeLists.txt` 按子目录追加源文件；新增模块仅依赖已有 `SequenceDocument`、`SequenceEditorContext`、Command、Registry，**不**反向依赖具体 ImGui Widget 实现（除 Widget 自身 TU）。

---

### B.4 第一优先级：`SequenceDocumentViewModel` 与 `SequenceEntryViewModel`

#### B.4.1 `SequenceDocumentViewModel`（第三阶段核心）

**建议公开能力**（名称可按项目命名风格微调，语义保持不变）：

```cpp
// 伪代码级 API 说明 — 实现时改为正式接口并补全 const 正确性、线程与生命周期注释
void Rebuild(SequenceDocument& document);
void ApplySearchFilter(/* 由 SequenceSearchViewModel 或独立 FilterSpec 提供 */);
void ApplyRuntimeOverlay(const SequenceRuntimeOverlayState& state);
void ApplyValidation(const std::vector<SequenceValidationIssue>& issues);

const std::vector<SequenceEntryViewModel>& GetVisibleEntries() const;
```

**`Rebuild` 语义（实现计划注释）**：

- 从 `SequenceDocument` 读取**有序条目列表**的权威顺序（与 Command 修改后的顺序一致）。
- 为每条构造 `SequenceEntryViewModel`：**EntryIndex** 与 Document 中索引一致，供 Command 参数使用。
- 合并 **Metadata**（图标、分类、显示名）与 **可选本地化**；不在此处做重量级资源 IO（缺资源由 Validator 报告）。

**增量更新（后续优化，不阻塞 v1）**：

- 首版可全量 `Rebuild`；条目数大时再引入「脏区间」或版本号，与 Virtualized UI 配合。

#### B.4.2 `SequenceEntryViewModel`

**字段建议**（与产品 UI 一致即可扩展）：

| 字段 | 含义 |
|------|------|
| `EntryIndex` | 与 Document 条目索引一致，用于命令与 Issue 关联 |
| `DisplayName` / `Subtitle` / `Category` / `Icon` | 纯展示；来源于 Metadata + 组件摘要 |
| `Selected` | 可由 Document VM 根据 `SequenceSelectionModel` 填充，或由 Widget 只读绑定 |
| `RuntimeActive` | 当前执行条目高亮 |
| `HasValidationError` | 是否存在至少一条绑定本 Entry 的 Issue（可按严重级别区分） |
| `HiddenByFilter` | 若希望「数据仍在但 UI 隐藏」则 true；否则过滤后不出现在 `GetVisibleEntries()` |

> **说明**：**禁止**在 `SequenceEntryViewModel` 中暴露原始 `IVGSSequenceComponent*` 给 Widget；若 Inspector 需要编辑，仍通过现有 Selection → Document → Inspector 路径，与第三阶段并行不悖。

---

### B.5 Timeline v1：范围明确（避免做成 Sequencer）

**允许（v1）**：

- 行布局、滚动、Hover、**重排**（经 Command）、**选择**、**过滤**、**Runtime 高亮**、**叠加层**（选区 / 运行时 / 校验提示）。

**禁止（v1）**：

- 曲线、关键帧、时间缩放（Zoom）、多轨、嵌套轨、音频波形、电影镜头轨等「剪辑器」语义。

> **说明**：Timeline v1 的定位是 **Sequence 的可视化 + 交互壳**，与线性 Runtime 对齐；为第四阶段 Graph 预留扩展点时，仅通过 **接口与数据边界** 扩展，不在 v1 实现图语义。

---

### B.6 `SequenceTimelineSelection` 与全局 `SequenceSelectionModel`

**计划**：

- **单一事实来源**：全局选中仍以现有 `SequenceSelectionModel`（或等价物）为准。
- `SequenceTimelineSelection` 表示 **视口内** 的拖拽框选、键盘焦点行等；提交时 **同步** 到全局 Selection，避免两套选中状态长期分叉。
- 文档中在 `SequenceTimelineController` 头文件用中文注释写清：**何时 commit 到全局 Selection**（鼠标释放、Enter、右键菜单打开前等）。

---

### B.7 Validation 系统

**`SequenceValidationIssue` 建议字段**：

- `SequenceValidationSeverity Severity`
- `uint32_t EntryIndex`（无条目关联可用哨兵值并在注释中说明）
- `std::string Message`（后续可扩展 `MessageId` + 参数化本地化）

**`SequenceValidationRegistry`**：

- 注册内置：`Missing Asset`、`Invalid Character`、`Invalid Background`、`Invalid Jump`、`Empty Dialogue`、`Duplicate ID`、`Runtime Incompatible State` 等；按迭代优先级逐个 PR 落地。
- 提供 `RunAll(SequenceDocument&)` 或 `RunAll(const SequenceDocument&)` + 可选只读缓存；**执行频率**由 ViewModel 在 `Rebuild` 或「保存前 / 手动刷新」触发，避免每帧全量。

**与 Inspector 关系**：

- Inspector **不再**作为错误唯一来源；可读取 Issue 列表做字段级高亮。

---

### B.8 Search Layer：`SequenceSearchViewModel`

**从「纯字符串过滤」升级为多维查询**（可分阶段）：

1. **类型 / TypeNameID** 过滤。
2. **Category** 过滤（与 Metadata 一致）。
3. **Validation** 过滤（仅显示有 Error/Warn 的条目）。
4. **Runtime 状态** 过滤（例如仅当前可执行、仅阻塞等待 — 具体语义与 Runtime 能力对齐）。

**输出**：作用于 `SequenceDocumentViewModel::ApplySearchFilter`，最终体现为 `GetVisibleEntries()` 与可选「匹配高亮片段」字段（后续）。

---

### B.9 Virtualized Rendering（虚拟化列表）

**动机**：条目数增大时，全量 ImGui 行绘制成为瓶颈。

**计划**：

- 在 `SequenceEntryListWidget`（或等价列表）中引入 **可见区域计算** → 仅对可见索引范围调用 `SequenceEntryViewModel` 绘制。
- 与 `Rebuild` 策略协调：**ViewModel 可保留全量逻辑条目**，虚拟化仅裁剪 **绘制**；滚动条高度仍反映过滤后总数。

> **说明**：首版可用「索引区间裁剪」；后续再抽象为通用虚拟列表助手（若项目内已有类似工具则复用）。

---

### B.10 分步迁移（带验收标准）

| 步骤 | 内容 | 验收标准 |
|------|------|----------|
| **1** | 新增 `SequenceDocumentViewModel` + `SequenceEntryViewModel`；旧 Widget **可 fallback** 直读 Document | 编译通过；开关或编译宏控制路径时行为与现网一致 |
| **2** | `SequenceEntryListWidget`（或当前主列表 Widget）改为使用 ViewModel | 列表展示、选择、滚动与改造前一致；**无** Document 内层循环出现在 Widget 源文件中 |
| **3** | 引入 `SequenceValidationRegistry` + 内置若干 `ISequenceValidator` | 能产出 Issue 列表；ValidationWidget 或状态栏可展示计数 |
| **4** | 引入 `SequenceRuntimeObserver` + `SequenceRuntimeOverlayState` | Widget 中无主动 Pull Runtime；试播时高亮随 Observer 更新 |
| **5** | `SequenceTimelineWidget` v1 | 满足 **B.5**（Timeline v1 允许/禁止清单）；重排走 Command |
| **6** | `SequenceOutlinerWidget` | 与同一 ViewModel 数据源一致；选中同步 |
| **7** | Virtualized Rendering | 大列表帧时间与行数弱相关（可用粗略性能对比记录于提交说明） |

---

### B.11 明确禁止项（第三阶段内）

- 现在 **Graph 化**、**Node Runtime**、**Pin System**、**ECS Timeline**、**Sequencer Clone**、**Branch Graph**。

**原因简述**：与当前**线性序列执行模型**不一致；会提前锁定错误的运行时契约，拖累 Presentation Layer 的稳定交付。

---

### B.12 第四阶段（预告，不在本阶段实现）

非线性叙事、分支可视化、任务图、执行流、分支 Runtime 等，在 **Presentation / Validation / Timeline / Runtime Overlay** 稳定后再评估；届时应复用：Document、Command、Registry、ViewModel 管道。

---

### B.13 建议开发顺序（PR / 分支粒度）

1. **PR-1**：`ViewModels/SequenceDocumentViewModel` + `SequenceEntryViewModel` + Document 变更订阅 + 单元测试（若有测试目标 `VGEditorGalgameSequenceTest` 则补最小用例）。
2. **PR-2**：列表 Widget 迁移 + 代码评审检查「无 Document 内层遍历」。
3. **PR-3**：`Validation` 目录 + 2～3 个高价值内置 Validator + `SequenceValidationWidget` 或状态栏摘要。
4. **PR-4**：`RuntimeObserver` + `OverlayState` + `SequenceRuntimeWidget` 或列表内嵌 Overlay。
5. **PR-5**：Timeline 核心（Controller + Layout + Viewport + Widget），功能裁剪在 v1 范围。
6. **PR-6**：Outliner + StatusBar（信息密度与性能再平衡）。
7. **PR-7**：Virtualized list + SearchViewModel 多维查询。

> **说明**：每个 PR 附带：**行为说明、与铁律对照自检表、已知限制**；文档 B 章节随实现微调 API 名称时同步更新本节表格与伪代码。

---

### B.14 与「组件注册」章节（A）的协同

- **Metadata**（`SequenceComponentMetadata`）是 EntryViewModel 展示字段的**首选来源**；Bootstrap 继续负责类型发现与默认展示。
- **新 Validator** 若依赖类型信息，应通过 **TypeNameID** 与运行时注册表对齐，避免重复硬编码类型列表。

---

**文档维护**：本 B 章节由第三阶段开发驱动更新；若 API 与目录有变更，请在本文件内保留「变更日期 + 简述」于次行注释式记录（可选）。

<!-- 维护记录模板（可选）：2026-05-11 — 初版：Presentation Layer 总体计划与迁移步骤。 -->
