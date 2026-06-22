# CSharpScriptAssetOpener 外部 IDE 集成

## 实施日期
2026-06-22

## 需求
双击 C# 脚本资产时，打开外部 IDE（Visual Studio / VS Code）加载整个解决方案（Game.sln）。

## 功能特性
- 支持 Visual Studio 和 VS Code
- 优先级：Visual Studio → VSCode
- 可配置：通过 Editor UI 设置面板选择首选 IDE
- 打开方式：打开整个解决方案（Game.sln）

## 修改文件清单

### 1. 新增文件

#### Neverness.Editor.Core
- `Public/IDEPreference.cs` - IDE 偏好枚举（VisualStudio / VSCode）
- `Public/IPreferencesService.cs` - 偏好设置服务接口
- `Private/PreferencesServiceImpl.cs` - 持久化到 `projectSettings/EditorPrefs.json`

#### Neverness.Editor.AvaloniaFrontend
- `Views/PreferencesWindow.axaml` - 设置面板 XAML
- `Views/PreferencesWindow.axaml.cs` - 设置面板逻辑

### 2. 修改文件

#### EditorApplicationRunner.cs
- 添加 `RegisterPreferencesService()` 方法
- 在 Phase 1.5 注册 `IPreferencesService`（必须在 AssetsModule 之前）

#### CSharpScriptAssetOpener.cs
- 实现 `OpenAsync` 方法
- 根据用户偏好选择 IDE
- Visual Studio：使用 `Process.Start` + `UseShellExecute = true`
- VS Code：调用 `code` 命令打开项目目录

#### EditorMenuItem.cs
- 新增 `CommandId` 字段，支持通过命令 ID 字符串查找命令

#### BuiltinMenuContributor.cs
- `Edit/Preferences...` 使用 `CommandId: "editor.preferences"` 绑定命令

#### MainEditorWindow.axaml.cs
- 菜单渲染时优先用 Command 对象，其次通过 CommandId 查找

#### AvaloniaFrontendModule.cs
- 注册 `editor.preferences` 命令（不重复注册菜单项）
- 添加 `OpenPreferencesWindow()` 方法

## 技术实现

### 依赖注入顺序
```
Phase 1:   EditorFrameworkModule.Install()
           EditorCoreModule.Install()

Phase 1.5: RegisterPreferencesService()  ← 新增，CSharpScriptAssetOpener 依赖

Phase 2:   AssetsModule.Install()  ← openerRegistry.Discover() 在这里
           SceneModule.Install()
           ScriptEditorModule.Install()

Phase 3:   AvaloniaFrontendModule.Install()  ← 注册菜单命令
```

**关键点**：`IPreferencesService` 必须在 `AssetsModule.Install()` 之前注册，否则 `AssetOpenerRegistry.Discover()` 无法实例化 `CSharpScriptAssetOpener`。

### 解决方案路径获取
```csharp
var projectRoot = VFS.GetAbsolutePath(ProjectPaths.Project.FullPath);
var slnPath = Path.Combine(projectRoot, "Game.sln");
```

### IDE 启动方式

#### Visual Studio
```csharp
var startInfo = new ProcessStartInfo
{
    FileName = slnPath,  // .sln 文件关联
    UseShellExecute = true
};
Process.Start(startInfo);
```

#### VS Code
```csharp
var startInfo = new ProcessStartInfo
{
    FileName = "code",
    Arguments = $"\"{projectRoot}\"",
    UseShellExecute = false,
    CreateNoWindow = true
};
Process.Start(startInfo);
```

### 配置持久化
配置文件位置：`projectSettings/EditorPrefs.json`

```json
{
  "PreferredIDE": "VisualStudio"
}
```

### 菜单命令绑定机制

#### EditorMenuItem CommandId 字段
`EditorMenuItem` 新增 `CommandId` 字段，当 `Command` 为 null 时，通过 `EditorMenuRegistry.FindCommand(CommandId)` 查找命令。

```csharp
// BuiltinMenuContributor 注册菜单项（使用 CommandId）
registry.Register(new EditorMenuItem("Edit/Preferences...",
    CommandId: "editor.preferences", SortOrder: 900, Icon: "🔧"));

// AvaloniaFrontendModule 注册命令
var preferencesCmd = new EditorCommand
{
    Id = "editor.preferences",
    Execute = _ => OpenPreferencesWindow()
};
CoreModuleImp.Context.Menus.RegisterCommand(preferencesCmd);
```

#### MainEditorWindow 菜单渲染
```csharp
// 优先 Command 对象，其次 CommandId 查找
var resolvedCommand = item.Command
    ?? (string.IsNullOrEmpty(item.CommandId) ? null : EditorMenuRegistry.FindCommand(item.CommandId));
```

## 工作流程
1. 用户双击 C# 脚本资产
2. `AssetOpenService.OpenAsync` 查找 TypeId=11 的 Opener
3. `CSharpScriptAssetOpener.OpenAsync` 被调用
4. 从 VFS 获取 Game.sln 物理路径
5. 读取 `IPreferencesService.PreferredIDE` 获取用户偏好
6. 启动对应的 IDE 打开解决方案

## 菜单入口
- `Edit → Preferences...` 打开设置面板
- 快捷键：无（可通过菜单访问）

## 编译状态
- Neverness.Editor.Core: ✅ 0 错误
- Neverness.Editor.Assets: ✅ 0 错误
- Neverness.Editor.AvaloniaFrontend: ✅ 0 错误
- NevernessEditor: ✅ 0 错误

## 调试过程中发现的问题

### 问题 1: AssetOpener 注册失败
**现象**：`[AssetOpenService] 无 Opener: /assets/NewScript.cs (TypeId=11)`

**根因**：`CSharpScriptAssetOpener` 构造函数需要 `IPreferencesService`，但 `AssetOpenerRegistry.Discover()` 在 `AssetsModule.Install()` 中调用时，`IPreferencesService` 还没注册。

**解决**：将 `IPreferencesService` 注册提前到 Phase 1.5（在 `AssetsModule.Install()` 之前）。

### 问题 2: 菜单项 Command 不生效
**现象**：点击 `Edit → Preferences...` 无任何响应

**根因**：
1. `BuiltinMenuContributor` 注册的 `Edit/Preferences...` 没有绑定 Command
2. 在 `AvaloniaFrontendModule` 中重新注册带 Command 的版本没有生效（菜单树缓存 + 时序问题）

**解决**：
1. `EditorMenuItem` 新增 `CommandId` 字段
2. `BuiltinMenuContributor` 使用 `CommandId: "editor.preferences"` 绑定
3. `MainEditorWindow` 菜单渲染时支持 CommandId 查找

## 注意事项
1. VS Code 需要 `code` 命令在系统 PATH 中
2. Visual Studio 依赖 .sln 文件关联（通常安装时自动配置）
3. Game.sln 由 `ScriptProjectGenerator` 自动生成
4. 配置文件在项目级别存储，每个项目独立
5. DLL 被 Visual Studio 锁定时需关闭 VS 再编译
