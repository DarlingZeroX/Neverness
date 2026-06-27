# NNRuntimeRmlui C# 迁移计划

> 从 C++ `NNRuntimeRmlui` 迁移到 C# `Neverness.Runtime.Rmlui`
> 创建日期：2026-06-27
> 更新日期：2026-06-27（Phase 1 完成）

---

## 1. 架构概述

### 1.1 分工

| 层 | 职责 | 实现方式 |
|---|------|----------|
| **C#** | 文档管理、输入处理、更新逻辑 | RmlUi.Net |
| **C++** | GPU 渲染 | RmlUIRenderer（暂不动） |
| **连接** | C# 把 Handle 传给 ViewportSurface | ABI: Create/Destroy |

### 1.2 架构图

```
┌─────────────────────────────────────────────────────────────┐
│                    C# 逻辑层                                │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │ RmlUISystem  │  │ Context      │  │ Document         │  │
│  │ 系统管理     │  │ RmlUi.Net    │  │ RmlUi.Net        │  │
│  └──────────────┘  └──────────────┘  └──────────────────┘  │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │ InputHandler │  │ Update       │  │ HotReloader      │  │
│  │ C# 实现      │  │ C# 实现      │  │ C# 实现          │  │
│  └──────────────┘  └──────────────┘  └──────────────────┘  │
└─────────────────────────┬───────────────────────────────────┘
                          │ Handle
┌─────────────────────────▼───────────────────────────────────┐
│                    ViewportSurface                           │
│  接收 Handle，调用 C++ RmlUIRenderer 渲染                   │
└─────────────────────────┬───────────────────────────────────┘
                          │
┌─────────────────────────▼───────────────────────────────────┐
│                    C++ RmlUIRenderer（暂不动）               │
│  内部包含：Rml::Context、RmlDiligentRenderInterface 等       │
│  只负责 GPU 渲染                                            │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. ABI 接口

### 2.1 C++ 暴露的接口（极简）

```cpp
// 只需要两个函数
extern "C" {
    NN_RMLUI_API uint32_t RmlRenderer_Create(int width, int height);
    NN_RMLUI_API void RmlRenderer_Destroy(uint32_t handle);
}
```

### 2.2 C# P/Invoke（LibraryImport）

```csharp
internal static partial class RmlNativeInterop
{
    private const string NativeLib = "NevernessRuntime-RmlUI";

    [LibraryImport(NativeLib, EntryPoint = "RmlRenderer_Create")]
    internal static partial uint RmlRenderer_Create(int width, int height);

    [LibraryImport(NativeLib, EntryPoint = "RmlRenderer_Destroy")]
    internal static partial void RmlRenderer_Destroy(uint handle);
}
```

---

## 3. 当前状态

### 3.1 Phase 1 已完成 ✅

| 文件 | 状态 | 说明 |
|------|------|------|
| `Internal/RmlNativeInterop.cs` | ✅ | LibraryImport P/Invoke |
| `Public/RmlRenderer.cs` | ✅ | Handle 封装 + RmlUi.Net 逻辑 |
| `Public/RmlDocument.cs` | ✅ | 文档封装 |
| `Public/RmlUISystem.cs` | ✅ | 全局系统管理 |

### 3.2 C# 结构

```
Neverness.Runtime.Rmlui/
├── Docs/
│   └── MIGRATION_PLAN.md
├── Internal/
│   └── RmlNativeInterop.cs     ← LibraryImport（Create/Destroy）
├── Public/
│   ├── RmlRenderer.cs          ← Handle 封装 + RmlUi.Net
│   ├── RmlDocument.cs          ← 文档封装
│   ├── RmlUISystem.cs          ← 全局系统
│   └── RmluiNativeApi.cs       ← 已有，保留
└── Neverness.Runtime.Rmlui.csproj
```

---

## 4. API 参考

### 4.1 RmlUISystem（静态门面）

```csharp
public static class RmlUISystem
{
    // 初始化/关闭
    public static void Initialize(int width, int height);
    public static void Shutdown();
    public static bool IsInitialized { get; }

    // 渲染器访问
    public static RmlRenderer? Renderer { get; }
    public static uint RendererHandle { get; }

    // 便捷方法
    public static RmlDocument? LoadDocument(string path);
    public static bool OnMouseMove(int x, int y, KeyModifier modifiers);
    public static bool OnMouseButton(MouseButton button, bool down, KeyModifier modifiers);
    public static bool OnMouseWheel(float delta, KeyModifier modifiers);
    public static bool OnKey(KeyIdentifier key, bool down, KeyModifier modifiers);
    public static bool OnTextInput(string text);
    public static void Update(float deltaTime);
    public static bool LoadFontFace(string path, bool fallback = false);
    public static void ReloadAllDocuments();
    public static void SetDebuggerVisible(bool visible);
}
```

### 4.2 RmlRenderer

```csharp
public sealed class RmlRenderer : IDisposable
{
    // 构造
    public RmlRenderer(int width, int height);

    // Handle（传给 ViewportSurface）
    public uint Handle { get; }

    // 文档管理
    public RmlDocument? LoadDocument(string path);
    public void ReloadAllDocuments();

    // 输入处理
    public bool OnMouseMove(int x, int y, KeyModifier modifiers);
    public bool OnMouseButton(MouseButton button, bool down, KeyModifier modifiers);
    public bool OnMouseWheel(float delta, KeyModifier modifiers);
    public bool OnKey(KeyIdentifier key, bool down, KeyModifier modifiers);
    public bool OnTextInput(string text);

    // 更新
    public void Update(float deltaTime);

    // 字体
    public bool LoadFontFace(string path, bool fallback = false);

    // 调试
    public void SetDebuggerVisible(bool visible);
    public bool IsDebuggerVisible { get; }
}
```

### 4.3 RmlDocument

```csharp
public sealed class RmlDocument : IDisposable
{
    // 属性
    public string Path { get; }
    public bool IsValid { get; }

    // 文档操作
    public void Show();
    public void Hide();
    public void Reload();
    public Element? GetElementById(string id);
    public string GetTitle();
    public void SetTitle(string title);
    public ElementDocument? GetDocument();

    // 事件
    public void AddEventListener(string eventName, EventListener listener, bool inCapturePhase = false);
    public void RemoveEventListener(string eventName, EventListener listener, bool inCapturePhase = false);
}
```

---

## 5. 使用示例

### 5.1 基本使用

```csharp
// 初始化
RmlUISystem.Initialize(1920, 1080);

// 加载字体
RmlUISystem.LoadFontFace("/fonts/Lato-Regular.ttf");

// 加载文档
var doc = RmlUISystem.LoadDocument("/ui/main.rml");
doc?.Show();

// 游戏循环
while (running)
{
    // 处理输入
    RmlUISystem.OnMouseMove(mouseX, mouseY, KeyModifier.None);
    RmlUISystem.OnKey(KeyIdentifier.KI_A, true, KeyModifier.None);

    // 更新
    RmlUISystem.Update(deltaTime);

    // 渲染（传给 ViewportSurface）
    ViewportSurface_RenderRml(surface, RmlUISystem.RendererHandle);
}

// 清理
RmlUISystem.Shutdown();
```

### 5.2 文档操作

```csharp
// 加载文档
var doc = RmlUISystem.LoadDocument("/ui/main.rml");
if (doc != null)
{
    doc.Show();

    // 获取元素
    var button = doc.GetElementById("myButton");

    // 设置标题
    doc.SetTitle("My UI");

    // 热重载
    doc.Reload();
}
```

### 5.3 输入处理

```csharp
// 鼠标
RmlUISystem.OnMouseMove(x, y, modifiers);
RmlUISystem.OnMouseButton(MouseButton.MB_LEFT, true, modifiers);
RmlUISystem.OnMouseWheel(delta, modifiers);

// 键盘
RmlUISystem.OnKey(KeyIdentifier.KI_SPACE, true, modifiers);
RmlUISystem.OnTextInput("Hello");
```

---

## 6. 待实施阶段

### Phase 2: 逻辑完善（预计 2 天）

- [ ] 完善热重载逻辑
- [ ] 完善调试器集成
- [ ] 添加更多文档操作

### Phase 3: ViewportSurface 集成（预计 2 天）

- [ ] 添加 ViewportSurface 调用接口
- [ ] 渲染循环集成
- [ ] 测试验证

### Phase 4: 系统集成（预计 2 天）

- [ ] ECS 组件集成
- [ ] 编辑器集成
- [ ] 功能测试

---

## 7. 时间估算

| 阶段 | 预计时间 | 状态 |
|------|----------|------|
| Phase 1: C# 桥接层 | 1 天 | ✅ 完成 |
| Phase 2: 逻辑完善 | 2 天 | 待开始 |
| Phase 3: ViewportSurface 集成 | 2 天 | 待开始 |
| Phase 4: 系统集成 | 2 天 | 待开始 |
| **总计** | **7 天** | |

---

## 8. 注意事项

1. **C++ 端暂不动** - 后面再精简
2. **RmlUi.Net 已修复** - 第三方库编译问题已解决
3. **Handle 只用于渲染** - 逻辑全部在 C# 实现
4. **LibraryImport** - 使用 .NET 7+ 源生成器
