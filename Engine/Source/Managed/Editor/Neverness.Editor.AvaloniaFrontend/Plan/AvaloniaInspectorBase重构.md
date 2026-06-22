# AvaloniaInspectorBase 重构

**日期**: 2026-06-22
**类型**: UI 组件库拆分重构

## 1. 背景

`AvaloniaInspectorBase.cs` 原文件 495 行，包含多种职责：
- 颜色常量（14 个 SolidColorBrush）
- 抽象接口（DisplayName、CanInspect、CreateInspector）
- 可折叠面板（CreateCollapsiblePanel）
- 属性行（CreatePropertyRow）
- 三轴输入（CreateVector3Row、CreateAxisInput）
- 颜色行（CreateColorRow）
- Slider 行（CreateSliderRow）— 未使用
- 资产拖拽（CreateAssetDropTarget）
- 数值输入（CreateNumericInput、CreateIntegerInput）
- 下拉框（CreateComboBox）

## 2. 拆分目标

- **Inspector 专用基类** vs **Editor Property UI 基础库** 分离
- 这些控件未来会被 MaterialEditor、AssetEditor、ProjectSettings 等非 Inspector 场景使用
- 颜色常量需要全编辑器统一

## 3. 最终结构

```
Neverness.Editor.AvaloniaFrontend/

├─ Inspectors/
│   └─ AvaloniaInspectorBase.cs    ~60 行  抽象接口 + CreateCollapsiblePanel
│
├─ PropertyEditor/
│   ├─ PropertyRows.cs             ~40 行  CreatePropertyRow
│   ├─ NumericFields.cs            ~90 行  CreateFloat/CreateUInt/CreateCombo
│   ├─ VectorFields.cs             ~130 行 CreateVector3/CreateColor
│   └─ AssetReferenceField.cs      ~110 行 Create（资产拖拽）
│
└─ Styling/
    └─ EditorTheme.cs              ~40 行  统一颜色常量
```

## 4. 各文件职责

### 4.1 AvaloniaInspectorBase.cs（Inspector 基类）

**职责**: 定义 Inspector 抽象接口，提供 Inspector 专属的 UI 方法

**保留内容**:
- `DisplayName` - 显示名称
- `CanInspect()` - 是否能检查指定类型
- `CreateInspector()` - 创建检查器 UI
- `CreateCollapsiblePanel()` - 可折叠组件面板（Inspector 专属）

**删除内容**:
- 所有颜色常量 → EditorTheme
- CreatePropertyRow → PropertyRows
- CreateVector3Row/CreateColorRow → VectorFields
- CreateNumericInput/CreateIntegerInput/CreateComboBox → NumericFields
- CreateAssetDropTarget → AssetReferenceField
- CreateSliderRow → 删除（死代码）

### 4.2 EditorTheme.cs（统一主题）

**职责**: 全编辑器共用的颜色常量

**颜色分组**:
- 背景色：Background、PanelBackground、HeaderBackground、InputBackground、HoverBackground
- 边框色：Border、BorderAccent
- 文字色：TextPrimary、TextSecondary、TextDisabled
- 轴标签色：AxisX/Y/Z、AxisR/G/B/A
- 状态色：Accent、Success、Warning、Error

### 4.3 PropertyRows.cs（属性行）

**职责**: 标签 + 编辑器的通用布局

**核心方法**:
```csharp
public static Control Create(string label, Control editor)
```

### 4.4 NumericFields.cs（数值输入）

**职责**: NumericUpDown 控件工厂

**核心方法**:
- `CreateFloat()` - 浮点数输入框
- `CreateUInt()` - 无符号整数输入框
- `CreateCombo()` - 下拉框

### 4.5 VectorFields.cs（向量/颜色）

**职责**: Vector3、Color 等多轴输入控件

**核心方法**:
- `CreateVector3()` - 三轴并排输入行
- `CreateColor()` - RGBA 颜色行

### 4.6 AssetReferenceField.cs（资产引用）

**职责**: 可接收资产拖拽的引用控件

**核心方法**:
```csharp
public static Control Create(string placeholderText, Action<string>? onAssetDropped, ...)
```

**复用场景**: Material Slot、Texture Slot、Audio Slot、Prefab Slot

## 5. 已更新的 Inspector

| Inspector | 更新内容 |
|-----------|----------|
| SpriteRendererInspector | 使用 PropertyRows、VectorFields、NumericFields、AssetReferenceField |
| RmlUIDocumentInspector | 使用 PropertyRows、AssetReferenceField |
| ScriptInspector | 使用 PropertyRows、AssetReferenceField、EditorTheme |
| TransformInspector | 使用 EditorTheme 颜色 |
| CameraInspector | 使用 EditorTheme 颜色 |

## 6. 删除的代码

**CreateSliderRow** — 历史遗留，当时预想用于 SpriteRenderer，后来发现 DragFloat 更合适。已删除。

## 7. 编译验证

- ✅ 0 错误
- ✅ 所有 Inspector 正常工作
- ✅ 颜色常量统一到 EditorTheme

## 8. 后续扩展点

1. **MaterialEditor** — 直接使用 PropertyRows、AssetReferenceField
2. **AssetEditor** — 直接使用 PropertyRows、NumericFields
3. **ProjectSettings** — 直接使用 PropertyRows、NumericFields、VectorFields
4. **自定义属性编辑器** — 组合使用 PropertyEditor 命名空间下的控件
