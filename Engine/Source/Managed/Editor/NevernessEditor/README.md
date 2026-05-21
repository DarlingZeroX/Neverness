# NervernessEditor（首包）

## 目标

通过加载 **NevernessRuntime-Managed.dll** 获取 **layout v7** Native 函数表（**NNApplicationAPI** + **NNWindowAPI**），驱动 `ApplicationHost` / `WindowHost` 显示**空应用窗口**，并在关闭窗口前持续泵送事件与推进托管 `RuntimeMainLoop`。

## 依赖

| 文件 | 说明 |
|------|------|
| `NevernessRuntime-Managed.dll` | 须以 `NEVERNESS_USE_ENGINE_RUNTIME_SERVICES=1` 构建（CMake 目标 `NevernessRuntime-Managed`） |
| `SDL3.dll` | 与 exe 同目录，或位于 PATH |

环境变量：

- `NEVERNESS_NATIVE_MANAGED_DLL`：显式指定 Managed DLL 绝对路径（可选）

## 构建步骤

1. CMake 配置并构建 Native：

   ```powershell
   cmake -S Engine -B Engine/Build -DCMAKE_BUILD_TYPE=Debug
   cmake --build Engine/Build --target NevernessRuntime-Managed -j
   ```

2. 构建 Editor（PostBuild 会尝试从 `Build/bin/$(Configuration)` 或 `Engine/Build/bin/$(Configuration)` 复制 DLL）：

   ```powershell
   dotnet build Neverness.slnx -c Debug
   ```

3. 若 SDL3 未自动复制，将 vcpkg 的 `SDL3.dll` 放到 `Build/bin/Debug/`（与 `NervernessEditor.exe` 同目录）。

## 运行

```powershell
.\Build\bin\Debug\NervernessEditor.exe
.\Build\bin\Debug\NervernessEditor.exe --title "My Editor" --width 1920 --height 1080
```

## 主循环时序

1. `NativeApiTableLoader` → `NNNativeApi_GetDefaultTable`
2. `RuntimeBootstrap.Start`（NativeDriven）
3. `ApplicationHost.Initialize` → `WindowHost.Create`
4. `while (PumpEvents)` → `BeginFrame` → `RuntimeMainLoop.Tick` → `EndFrame`
5. `ApplicationHost.Shutdown` → `RuntimeBootstrap.Shutdown`

## 与其它入口的区别

- **Neverness.Runtime.Host**：UCO 入口，供 Native hostfxr 调用 `Entry.Bootstrap` / `RuntimeTick`
- **VGEditor + NEVERNESS_USE_RUNTIME_KERNEL**：C++ 旧编辑器路径
- **本 exe**：纯托管 Main + Editor 侧 `NativeLibrary` 加载 Managed DLL（Runtime 程序集仍禁止 DllImport）
