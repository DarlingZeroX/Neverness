# 23. CodeEditor 模块实施计划（AvaloniaEdit）

**日期**：2026-06-24
**状态**：Phase 0-5 已完成，Phase 6（自定义主题文件）待实施
**模块**：Neverness.Editor.CodeEditor
**NuGet**：Avalonia.AvaloniaEdit 12.0.0 + AvaloniaEdit.TextMate 12.0.0

---

## 1. 目标

新建 `Neverness.Editor.CodeEditor` 模块，基于 AvaloniaEdit 实现编辑器内代码编辑功能。

**核心场景**：RmlUI 页面开发（HTML/CSS/RML/RCSS），同时支持 .cs/.hlsl/.glsl 等代码文件。

**用户故事**：
- 双击 Content Browser 中的 `.html`/`.rml`/`.css`/`.rcss` 文件 → 在 Dock 标签页中打开代码编辑器
- 双击 `.cs` 文件 → 默认打开外部 IDE，可通过设置切换为内联编辑
- 编辑器支持语法高亮（TextMate）、行号、代码折叠、多光标、搜索/替换
- Ctrl+S 保存修改后的文件

---

## 2. 架构概览

```
┌─────────────────────────────────────────────────────────────┐
│  Neverness.Editor.CodeEditor (新模块)                        │
│                                                             │
│  Public/                                                    │
│    CodeEditorModule.cs          ← Install() 入口            │
│    ICodeEditorService.cs        ← 服务接口                  │
│    CodeEditorAssetOpener.cs     ← [AssetOpener] 注册        │
│  Private/                                                   │
│    CodeEditorServiceImpl.cs     ← 服务实现                  │
│    Views/                                                   │
│      CodeEditorView.axaml       ← AvaloniaEdit 控件         │
│      CodeEditorView.axaml.cs                              │
│    Syntax/                                                  │
│      TextMateGrammarLoader.cs   ← 加载 .tmlanguage + 主题   │
│  Resources/                                                 │
│    Grammars/                    ← 内嵌语法文件               │
│      html.tmLanguage.json                                 │
│      css.tmLanguage.json                                  │
│      javascript.tmLanguage.json                           │
│    Themes/                      ← 内嵌主题                  │
│      dark.tmTheme.json                                    │
│      light.tmTheme.json                                   │
└──────────────────────┬──────────────────────────────────────┘
                       │ 依赖
        ┌──────────────┼──────────────┐
        ▼              ▼              ▼
  Editor.Framework  Editor.Assets  Editor.AvaloniaFrontend
                                            │
                                    ┌───────┴───────┐
                                    │ Dock 集成      │
                                    │ EditorDockFactory  ← 新增 CodeEditor 文档创建
                                    │ DockableAssetEditorFramework ← 新增 OpenCodeEditor
                                    │ AvaloniaFrontendModule ← 注册 CodeEditorAssetOpener
                                    └───────────────┘
```

---

## 3. 文件清单

### 3.1 新建文件（Neverness.Editor.CodeEditor 模块内）

| # | 文件完整路径 | 说明 |
|---|---|---|
| 1 | `Engine/Source/Managed/Editor/Neverness.Editor.CodeEditor/Neverness.Editor.CodeEditor.csproj` | 项目文件，NuGet: Avalonia.AvaloniaEdit 12.0.0 + AvaloniaEdit.TextMate 12.0.0 |
| 2 | `Engine/Source/Managed/Editor/Neverness.Editor.CodeEditor/Public/CodeEditorModule.cs` | 模块入口，Install() 初始化 TextMateGrammarLoader + 注册 ICodeEditorService |
| 3 | `Engine/Source/Managed/Editor/Neverness.Editor.CodeEditor/Public/ICodeEditorService.cs` | 服务接口（OpenFile / SaveFile / CloseAll / IsFileDirty） |
| 4 | `Engine/Source/Managed/Editor/Neverness.Editor.CodeEditor/Private/CodeEditorServiceImpl.cs` | 服务实现，管理打开文件状态 + VFS 读写 |
| 5 | `Engine/Source/Managed/Editor/Neverness.Editor.CodeEditor/Private/Views/CodeEditorView.axaml` | AvaloniaEdit 控件 AXAML（行号 + 工具栏 + 状态栏） |
| 6 | `Engine/Source/Managed/Editor/Neverness.Editor.CodeEditor/Private/Views/CodeEditorView.axaml.cs` | 控件后台（Ctrl+S 保存、脏标记、光标位置、语言识别） |
| 7 | `Engine/Source/Managed/Editor/Neverness.Editor.CodeEditor/Private/Syntax/TextMateGrammarLoader.cs` | TextMate 语法目录管理（外部目录优先 → 内嵌资源释放） |
| 8 | `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/AssetOpening/CodeEditorAssetOpener.cs` | IAssetOpener 实现，反射安装 TextMate，Dock 标签页集成 |

### 3.2 修改文件（现有模块）

| # | 文件完整路径 | 改动说明 |
|---|---|---|
| 1 | `Engine/Source/Managed/Editor/Neverness.Editor.Assets/Import/ImportResult.cs` | 新增 `AssetTypeId.HtmlDocument = 12`（第 93 行附近） |
| 2 | `Engine/Source/Managed/Editor/Neverness.Editor.Assets/Import/MetaFileManager.cs` | InferAssetTypeId 新增 `.html/.rml/.css/.rcss/.js → 12`（第 384 行附近） |
| 3 | `Engine/Source/Managed/Editor/Neverness.Editor.Settings/Private/Descriptors/EditorPreferencesSettings.cs` | 新增 `CsEditorMode` 枚举 + `CsEditorMode` 字段 |
| 4 | `Engine/Source/Managed/Editor/Neverness.Editor.Settings/Private/Descriptors/EditorPreferencesSettingsDescriptor.cs` | 新增 CsEditorMode FieldDescriptor（Order = 5） |
| 5 | `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Dock/EditorDockFactory.cs` | 新增 `CodeEditorPrefix` 常量 + `_codeEditorDocuments` 字典 + `CreateCodeEditorDocument()` + `RemoveCodeEditorPanel()` + `FindDocument` 扩展 |
| 6 | `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Dock/DockableAssetEditorFramework.cs` | 新增 `OpenCodeEditor()` / `CloseCodeEditor()` / `GetCodeEditorPanelId()` |
| 7 | `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Public/AvaloniaFrontendModule.cs` | `RegisterAssetOpeners()` 新增 CodeEditorAssetOpener 注册（第 198 行附近） |
| 8 | `Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/Neverness.Editor.AvaloniaFrontend.csproj` | 新增 CodeEditor ProjectReference |
| 9 | `Engine/Source/Managed/Editor/NevernessEditor/EditorApplicationRunner.cs` | Phase 2 新增 `CodeEditorModule.Install()` 调用（第 93 行附近） |
| 10 | `Engine/Source/Managed/Editor/NevernessEditor/NevernessEditor.csproj` | 新增 CodeEditor ProjectReference |

---

## 4. 详细设计

### 4.1 支持的文件扩展名与 TypeId

| 扩展名 | TypeId | 语法高亮 | 说明 |
|---|---|---|---|
| `.html`, `.htm` | 12 (HtmlDocument) | HTML | RmlUI 页面主文件 |
| `.rml` | 12 (HtmlDocument) | HTML | RmlUI 标记文件 |
| `.css` | 12 (HtmlDocument) | CSS | 样式表 |
| `.rcss` | 12 (HtmlDocument) | CSS | RmlUI 样式表 |
| `.js` | 12 (HtmlDocument) | JavaScript | 脚本文件 |
| `.cs` | 11 (CSharpScript) | C# | 受 EditorPreferences 控制 |
| `.hlsl`, `.glsl` | 5 (Shader) | GLSL/HLSL | 着色器文件（TextMate grammar 可选） |

**.cs 文件的特殊处理**：
- 检查 `EditorSettings.Preferences.CsEditorMode` 设置
- `ExternalIDE`（默认）→ 走原 `CSharpScriptAssetOpener` 逻辑
- `InlineEditor` → 走 `CodeEditorAssetOpener` 逻辑
- 由于两个 Opener 都注册了 TypeId 11，需在 `CodeEditorAssetOpener.CanOpen()` 中判断设置

### 4.2 CodeEditorAssetOpener

```csharp
[AssetOpener(AssetTypeId.HtmlDocument)]  // 12
public sealed class CodeEditorAssetOpener : IAssetOpener
{
    private readonly ICodeEditorService _editorService;
    private readonly AssetEditorManager _editorManager;

    // 支持 TypeId 12（HTML）和 TypeId 5（Shader）
    // TypeId 11（CSharpScript）在 AvaloniaFrontendModule 中手动注册
    public bool CanOpen(ulong assetTypeId)
        => assetTypeId is AssetTypeId.HtmlDocument or AssetTypeId.Shader;

    public async Task OpenAsync(AssetOpenContext context)
    {
        // 1. 检查是否已打开 → 激活现有标签
        // 2. 从 VFS 读取文件文本
        // 3. 通过 UI 线程打开 CodeEditorView
    }
}
```

**手动注册**（在 AvaloniaFrontendModule.RegisterAssetOpeners 中）：
```csharp
// CodeEditor 也处理 .cs（当设置为内联编辑时）
openerRegistry.Register(new CodeEditorAssetOpener(editorManager, codeEditorService));
```

### 4.3 CodeEditorView（AvaloniaEdit 控件）

```xml
<!-- CodeEditorView.axaml -->
<UserControl xmlns="https://github.com/avaloniaui">
    <DockPanel>
        <!-- 工具栏 -->
        <StackPanel DockPanel.Dock="Top" Orientation="Horizontal">
            <Button Content="Save" Command="{Binding SaveCommand}" />
            <TextBlock Text="{Binding FileName}" />
            <TextBlock Text="{Binding CursorPosition}" />
        </StackPanel>

        <!-- AvaloniaEdit 主体 -->
        <avaloniaEdit:TextEditor
            Name="Editor"
            ShowLineNumbers="True"
            SyntaxHighlighting="{Binding Highlighting}"
            FontFamily="Cascadia Code,Consolas,monospace"
            FontSize="14" />
    </DockPanel>
</UserControl>
```

**功能清单**：
- [x] 行号显示
- [x] TextMate 语法高亮（HTML/CSS/JS/C#）
- [x] 代码折叠
- [x] 多光标编辑（Ctrl+D 选中下一个匹配）
- [x] 搜索/替换（Ctrl+F / Ctrl+H）
- [x] 撤销/重做（Ctrl+Z / Ctrl+Y）
- [x] 自动缩进
- [x] 保存（Ctrl+S → 写回 VFS）
- [x] 未保存标记（标题栏 `*`）
- [x] 状态栏（行号:列号）

### 4.4 TextMate 语法加载

```csharp
internal sealed class TextMateGrammarLoader
{
    private TextMate.Installation? _installation;

    // 从内嵌资源加载 .tmLanguage.json
    public IHighlightingDefinition LoadGrammar(string languageName);

    // 从内嵌资源加载 .tmTheme.json
    public ThemeData LoadTheme(string themeName);

    // 安装到 TextEditor 控件
    public void Apply(TextEditor editor, string language, string theme);

    // 扫描外部目录加载额外语法
    // 路径：{EditorResources}/Grammars/*.tmLanguage.json
    public IReadOnlyList<IHighlightingDefinition> LoadExternalGrammars(string directory);
}
```

**内置语法文件来源**：
- TextMate 语法仓库：https://github.com/textmate/textmate.tmbundle
- 或使用 AvaloniaEdit 官方示例中的 .tmLanguage.json
- 只需内嵌 HTML / CSS / JavaScript 三个基础语法

**外部语法扩展**：
- 用户可将 `.tmLanguage.json` 和 `.tmTheme.json` 放到 `{ProjectSettings}/EditorResources/Grammars/` 和 `{ProjectSettings}/EditorResources/Themes/`
- 启动时扫描加载，覆盖/补充内置语法

### 4.5 Dock 集成（EditorDockFactory 改动）

```csharp
// EditorDockFactory.PanelIds 新增
public const string CodeEditorPrefix = "CodeEditor_";

// 新增字段
private readonly Dictionary<string, Document> _codeEditorDocuments = new();

// 新增方法
public Document CreateCodeEditorDocument(string fileName, Guid guid)
{
    var panelId = $"{PanelIds.CodeEditorPrefix}{guid}";
    var document = new Document
    {
        Id = panelId,
        Title = fileName,
        CanFloat = true,
        CanClose = true,
    };
    _codeEditorDocuments[panelId] = document;
    return document;
}

// FindDocument 扩展
public Document? FindDocument(string panelId)
{
    if (_viewport?.Id == panelId) return _viewport;
    if (_textureViewerDocuments.TryGetValue(panelId, out var tv)) return tv;
    if (_codeEditorDocuments.TryGetValue(panelId, out var ce)) return ce;
    return null;
}
```

### 4.6 DockableAssetEditorFramework 扩展

```csharp
public Document OpenCodeEditor(string assetName, GUID assetGuid, Control content)
{
    var editorId = ToEditorId(assetGuid);
    var panelId = $"{EditorDockFactory.PanelIds.CodeEditorPrefix}{editorId:D}";

    if (_dockFactory.CodeEditorDocuments.TryGetValue(panelId, out var existing))
    {
        existing.Context ??= content;
        _mainWindow.ShowDocument(existing, (Control)existing.Context!);
        _assetEditorManager.RegisterEditor(assetGuid, editorId);
        return existing;
    }

    var document = _dockFactory.CreateCodeEditorDocument(assetName, editorId);
    _mainWindow.ShowDocument(document, content);
    _assetEditorManager.RegisterEditor(assetGuid, editorId);
    return document;
}
```

### 4.7 EditorPreferencesSettings 扩展

```csharp
// 新增枚举
public enum CsEditorMode
{
    ExternalIDE,    // 默认：VS / VSCode
    InlineEditor    // 内联代码编辑器
}

// EditorPreferencesSettings 新增字段
[SettingField(DisplayName = ".cs 文件编辑方式")]
public CsEditorMode CsEditorMode { get; set; } = CsEditorMode.ExternalIDE;
```

### 4.8 文件保存流程

```
用户按 Ctrl+S
  → CodeEditorView 触发 SaveCommand
    → ICodeEditorService.SaveFile(virtualPath, text)
      → VFSService.WriteText(virtualPath, text)   // 写回 VFS
      → 清除脏标记，更新标题栏（去掉 *）
```

**不走 AssetPipeline 导入**：代码文件（HTML/CSS/CS）是纯文本，不需要二进制导入。直接 VFS 读写。

---

## 5. 实施阶段

### Phase 0：项目骨架 + NuGet 引用（0.5 天） ✅

- [x] 创建 `Neverness.Editor.CodeEditor.csproj`
- [x] 添加 NuGet：`Avalonia.AvaloniaEdit` 12.0.0、`AvaloniaEdit.TextMate` 12.0.0
- [x] 创建目录结构（Public/、Private/、Resources/）
- [x] 创建 `CodeEditorModule.cs` + `ICodeEditorService` + `CodeEditorServiceImpl`
- [x] 在 `EditorApplicationRunner.Install()` 中调用 `CodeEditorModule.Install()`

### Phase 1：TextMate 语法加载（1 天） ✅

- [x] VFS 语法文件：`/editor/codeEditor/{html,css,csharp}/syntaxes/*.tmLanguage.json`
- [x] 实现 `TextMateGrammarLoader`（VFS `ReadText` → 临时文件 → `Installation.SetGrammarFile()`）
- [x] `TextMateGrammarLoader.Install()` 在 TextEditor 上安装语法高亮
- [x] CodeEditorAssetOpener 调用 `_grammarLoader.Install()` 为每个文件安装语法
- [x] `DarkPlusTheme` 实现 `IRawTheme` 接口（TextMateSharp 2.0.3 无内置主题类）
- [x] Injection 机制：HTML 中 `<style>`/`<script>` 内的 CSS/JS 自动高亮
- [x] 加载顺序：先 CSS/JS → 再 HTML（确保 injection 注册表可用）
- [ ] 自定义 `.tmTheme.json` 主题文件（待补充，当前使用硬编码 DarkPlus 颜色）

### Phase 2：CodeEditorView 控件（1 天） ✅

- [x] 实现 `CodeEditorView.axaml`（AvaloniaEdit + 工具栏 + 状态栏）
- [x] 实现 `CodeEditorView.axaml.cs`（Ctrl+S 保存、脏标记、行号显示、语言识别）
- [x] 实现 `ICodeEditorService` / `CodeEditorServiceImpl`（文件打开/保存/关闭管理）

### Phase 3：AssetTypeId + AssetOpener 注册（0.5 天） ✅

- [x] `ImportResult.cs` 新增 `AssetTypeId.HtmlDocument = 12`
- [x] `MetaFileManager.InferAssetTypeId` 新增 `.html`/`.rml`/`.css`/`.rcss` 映射
- [x] 实现 `CodeEditorAssetOpener`（支持 TypeId 12/5/11，反射安装 TextMate）
- [x] `AvaloniaFrontendModule.RegisterAssetOpeners()` 注册 opener

### Phase 4：Dock 集成（0.5 天） ✅

- [x] `EditorDockFactory` 新增 `CodeEditorPrefix` + `CreateCodeEditorDocument()` + `FindDocument` 扩展
- [x] `DockableAssetEditorFramework` 新增 `OpenCodeEditor()` / `CloseCodeEditor()`
- [x] `AvaloniaFrontendModule.RegisterAssetOpeners()` 注册 CodeEditorAssetOpener

### Phase 5：.cs 可配置编辑模式（0.5 天） ✅

- [x] `EditorPreferencesSettings` 新增 `CsEditorMode` 枚举（ExternalIDE / InlineEditor）
- [x] `EditorPreferencesSettingsDescriptor` 新增对应 FieldDescriptor
- [x] `CodeEditorAssetOpener.CanOpen()` 对 TypeId 11 检查设置
- [ ] `CSharpScriptAssetOpener` 对 TypeId 11 检查设置（互相排斥，待验证）

### Phase 6：外部语法扩展 + HLSL/GLSL（0.5 天）

- [ ] 实现外部语法目录扫描：`{ProjectSettings}/EditorResources/Grammars/`
- [ ] 可选：内嵌 GLSL/HLSL 的 `.tmLanguage.json`
- [ ] 文档：如何添加自定义语法文件

---

## 6. 项目依赖关系

```xml
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <AssemblyName>Neverness.Editor.CodeEditor</AssemblyName>
    <TargetFramework>net10.0</TargetFramework>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>enable</Nullable>
    <RootNamespace>Neverness.Editor.CodeEditor</RootNamespace>
  </PropertyGroup>

  <ItemGroup>
    <!-- AvaloniaEdit 核心 + TextMate -->
    <PackageReference Include="Avalonia.AvaloniaEdit" Version="x.y.z" />
    <PackageReference Include="AvaloniaEdit.TextMate" Version="x.y.z" />
  </ItemGroup>

  <ItemGroup>
    <!-- 内嵌语法/主题资源 -->
    <EmbeddedResource Include="Resources\Grammars\**" />
    <EmbeddedResource Include="Resources\Themes\**" />
  </ItemGroup>

  <ItemGroup>
    <ProjectReference Include="..\Neverness.Editor.Framework\Neverness.Editor.Framework.csproj" />
    <ProjectReference Include="..\Neverness.Editor.Assets\Neverness.Editor.Assets.csproj" />
    <ProjectReference Include="..\Neverness.Editor.Core\Neverness.Editor.Core.csproj" />
    <ProjectReference Include="..\Neverness.Editor.Settings\Neverness.Editor.Settings.csproj" />
  </ItemGroup>
</Project>
```

**AvaloniaEdit 版本**：需确认与 Avalonia 12.0.4 兼容的版本号。AvaloniaEdit 从 0.10.x 开始支持 Avalonia 11+，需在 NuGet 上确认最新兼容版本。

---

## 7. 已知问题与修复

### 7.1 AvaloniaEdit TextEditor 透明/不显示

**现象**：TextEditor 控件创建成功，Text 已设置，但渲染区域全黑/透明。

**根因**：AvaloniaEdit 的 `TextEditor` 继承自 `TemplatedControl`，需要 Fluent 主题模板才能渲染。App.axaml 未导入 AvaloniaEdit 主题。

**修复**：在 `App.axaml` 的 `<Application.Styles>` 中添加：
```xml
<StyleInclude Source="avares://AvaloniaEdit/Themes/Fluent/AvaloniaEdit.xaml" />
```

正确的资源路径通过反编译 AvaloniaEdit.dll 找到：
- `avares://AvaloniaEdit/Themes/Base.xaml`
- `avares://AvaloniaEdit/Themes/Fluent/AvaloniaEdit.xaml` ← 使用此
- `avares://AvaloniaEdit/Themes/Simple/AvaloniaEdit.xaml`

**修改文件**：`Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/App.axaml`（第 20 行）

### 7.2 Dock Document Context 不渲染

**现象**：`document.Context = content` 设置后，Dock 标签页不显示内容。

**根因**：Dock 12.0 的 `FuncDataTemplate` 需要手动注册才能渲染 `Document.Context`。已在 `App.axaml.cs` 中通过 `DockContentHost` + Binding 解决。

**参考**：`Engine/Source/Managed/Editor/Neverness.Editor.AvaloniaFrontend/App.axaml.cs` 第 30-46 行。

---

## 8. 风险与待确认项

| # | 风险/待确认 | 影响 | 缓解措施 |
|---|---|---|---|
| 1 | AvaloniaEdit 与 Avalonia 12.0.4 的版本兼容性 | 阻塞 | Phase 0 先验证 NuGet 安装 + 编译 |
| 2 | TextMate .tmlanguage.json 文件来源和许可证 | 中 | 使用 MIT 许可的开源语法文件 |
| 3 | .cs 内联编辑时与 CSharpScriptAssetOpener 的 TypeId 11 冲突 | 中 | CanOpen 互斥判断，由设置控制优先级 |
| 4 | 大文件（>100KB）的编辑性能 | 低 | AvaloniaEdit 虚拟化渲染，天然支持大文件 |
| 5 | TextMateSharp 2.0.3 API 无 `RegistryOptions`/`ThemeName` 类 | 中 | 需实现 `IRegistryOptions` + `IRawTheme` 接口 |
| 6 | `IRegistryOptions.GetDefaultTheme()` 返回 null 导致 NullReferenceException | 高 | 必须返回非 null 的 `IRawTheme` 实例 |

---

## 8. 验收标准

1. 双击 Content Browser 中的 `.html` 文件 → 在 Dock 中央标签页打开代码编辑器
2. 语法高亮正确显示（HTML 标签、属性、CSS 规则）
3. 行号显示、代码折叠正常
4. Ctrl+S 保存文件，关闭后重新打开内容不丢失
5. 同一文件不会重复打开（已有则激活现有标签）
6. `.cs` 文件默认打开外部 IDE，切换设置后改为内联编辑
7. 外部目录放置 `.tmLanguage.json` 后重启编辑器可识别新语法
