注册新的 Galgame 序列组件 (运行时 + 编辑器)
本文档说明了如何使新的 IVGSSequenceComponent 类型在 VGEditorGalgameSequence 中变为可编辑状态，而无需手动维护重复的列表。

## 1. 运行时类型与工厂 (VGGalgameScriptSequence)
在脚本序列模块中定义组件（通常使用 TVGSSequenceComponent<YourType>）。

在 IVGSSequenceComponentManager 中进行注册（参考 IVGSSequenceComponent.cpp 中的 EmplaceComponentType<YourType>()）。
IVGSSequenceComponentManager::EnumerateRegisteredTypeNameIDs 是“存在哪些类型”的唯一事实来源。

## 2. 编辑器调色板与检查器引导程序 (Bootstrap)
BootstrapSequenceComponentRegistry 会遍历由 EnumerateRegisteredTypeNameIDs 返回的 所有 ID，并构建 SequenceComponentMetadata（分类默认为 序列组件，图标从 SequenceEntryUIDataManager 中获取，若有）。

随后，BootstrapSequenceInspectorRegistry 为每个元数据条目注册一个 LegacyDrawerInspectorAdapter。

## 3. UI 数据与遗留绘制器 (VGEditorGalgameSequence)
在 SequenceEntryUIDataManager（构造函数位于 EntryUIData.cpp）中：

为你的类型添加一行 SequenceEntryUIData 配置（包括标签 Label、FontAwesome 图标标签 IconLabel 以及可选的分类 Category）。

通过 GalSeqComDrawerRegistry::GetInstance().RegisterDrawer(MakeRef<YourDrawer>()) 注册 GalSeqComDrawer。

对于尚未迁移到专用 ISequenceInspector 的类型，绘制器（Drawer）将驱动遗留的检查器路径。

## 4. 支持撤销的检查器字段 (Undo-aware)
对于高频修改的字段：

扩展 SequenceEditFieldId / EditSequencePropertyCommand（参考 LegacyDrawerInspectorAdapter 中的 CommonDialogue 实现）。

或者，为该类型添加原生的 ISequenceInspector 实现，并注册它以替代该类型的遗留适配器（Legacy Adapter）。