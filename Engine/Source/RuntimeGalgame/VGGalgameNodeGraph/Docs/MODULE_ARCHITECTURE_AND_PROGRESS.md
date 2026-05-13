# VGGalgameNodeGraph 模块架构与进展

## 职责

- **DialogueList 数据模型**：`Include/DialogueListNodeData.h` — 对白行、表现参数、与 JSON 序列化/反序列化（供编辑器与运行时共用）。
- **节点执行函数**：`Include/VGNodeExec_Galgame.h` / `Source/VGNodeExec_Galgame.cpp` — 与 `HNGRuntimeCore` 的 `NodeExecuteFn` 签名一致的 `EntryExecute`、`DialogueListExecute`、`ChoiceExecute`、立绘/BGM/背景等入口；**不依赖编辑器 UI**。

## CMake

- 目标：`VGGalgameNodeGraph`（`SHARED`）。
- 链接：`PUBLIC HNGRuntimeCore`、`PUBLIC HCore`（头文件内联使用 `nlohmann::json`，消费方包含 `DialogueListNodeData.h` 时需传递 HCore 包含路径）。

## 典型消费方

- **`VGGalgame`**：`PUBLIC` 链接本库，保证运行时进程加载含节点执行符号的 DLL（与原先通过 `VGGalgameRuntime` 间接链接的行为一致）。
- **`VGEditorGalgame`**：节点注册与对白面板引用 `VGNodeExec_Galgame`、`DialogueListNodeData`。

## 进展

| 日期 | 说明 |
|------|------|
| 2026-05-13 | 自 **`VGGalgameRuntime`** 拆出独立模块 **`VGGalgameNodeGraph`**；剧情脚本工厂与 `StoryScriptSystem` 迁至 **`VGGalgameCore`** / **`VGGalgame`**；删除 **`VGGalgameRuntime`**。 |

## 历史摘要（自原 Runtime 文档迁移）

- **SubsystemBus**：对白继续等仍由宿主引擎与 `IStoryScriptSystem` 驱动；本模块仅负责节点图槽位与 `RuntimeContext.variables` 的约定键名（如 `Vars::CurrentText`）。
