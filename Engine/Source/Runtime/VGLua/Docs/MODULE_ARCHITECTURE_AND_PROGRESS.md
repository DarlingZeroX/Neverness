# VGLua — Lua 运行时（Lua + sol2）

## 1. 定位与边界

| 项目 | 说明 |
|------|------|
| **职责** | 将 **Lua** 以 **动态库** 形式集成（`LUA_BUILD_AS_DLL`），并 **PUBLIC** 暴露 `Include/` 下的 **Lua C API** 与 **sol2** 头文件，供 **VGCore**、**VGEngine**、**VGUI** 等做脚本绑定。 |
| **不负责** | 不负责游戏逻辑框架本身（见 **VGEngine**）；不负责 RmlUi 数据绑定（见 **VGUI**）。 |
| **CMake 目标** | `VGLua`（`SHARED`） |
| **依赖** | `HCore`（PRIVATE）。 |
| **典型消费者** | **VGCore**、**VGEngine**、**VGUI**（CMake 中常 `PUBLIC` 或显式添加 `VGLua/Include`）。 |

---

## 2. 构建与选项

| 项目 | 说明 |
|------|------|
| `LUA_BUILD_AS_DLL` | 通过 `add_definitions` 启用，Lua 以 DLL/DSO 导出符号。 |
| C 标准 | Lua 源码按 **C90** 编译（CMake 中 `CMAKE_C_STANDARD 90`）。 |
| 包含目录 | **PRIVATE** `Engine/Source/Runtime`、`Engine/Source/Kernel`；**PUBLIC** `Include`、`Include/Lua`；依赖方可 `#include <lua.h>` 与 `sol/...`。 |

---

## 3. 目录结构（摘要）

```
Engine/Source/Runtime/VGLua/
├── CMakeLists.txt
├── Docs/
│   └── MODULE_ARCHITECTURE_AND_PROGRESS.md   ← 本文件
├── Include/
│   ├── Lua/                   ← 官方 Lua 头（lapi.h、lua.h…）
│   ├── sol/                   ← sol2 头-only 绑定库
│   └── VGLuaCore/             ← 引擎侧 Lua 辅助（如错误管理）
├── Source/Lua/                ← Lua VM 源码 (.c)
└── Source/（若有 C++ 绑定实现）
```

**第三方内嵌**

- **Lua**：官方源码树；API 见 [Lua 5.4 手册](https://www.lua.org/manual/5.4/)（版本以 `lua.h` 中 `LUA_VERSION_*` 为准）。
- **sol2**：头文件库；文档见 [sol2](https://github.com/ThePhD/sol2)。

---

## 4. 使用说明

### 4.1 包含方式

```cpp
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include <sol/sol.hpp>
```

链接 **`VGLua`** 目标以解析 Lua VM 符号。

### 4.2 线程与 VM

- **单个 `lua_State*`** 默认 **非线程安全**；多线程须每线程一状态机或自行加锁。
- 宿主须在进程退出路径上 **`lua_close`**（由封装类负责，见 **VGCore**/**VGEngine** 中的 Helper）。

### 4.3 项目扩展头

| 头文件 | 说明 |
|--------|------|
| [`LuaErrorManager.h`](../Include/VGLuaCore/LuaErrorManager.h) | 错误收集/转发策略。 |
| [`LuaLocalizator.h`](../Include/VGLuaCore/LuaLocalizator.h) | 脚本侧本地化辅助。 |

---

## 5. 接口与 API 文档策略

- **Lua C API**：以官方手册为准；本仓库不维护逐函数副本。
- **sol2**：以模板元编程 API 为准；建议在业务代码中用 **窄封装**（如 `LuaHelper`）减少模板错误面。

---

## 6. 开发进展

| 日期 | 进展 |
|------|------|
| 2026-05-15 | 文档首版。 |

---

## 7. 相关链接

- [Runtime 总览](../../RUNTIME_ARCHITECTURE_AND_PROGRESS.md)
- [VGCore](../VGCore/Docs/MODULE_ARCHITECTURE_AND_PROGRESS.md)
