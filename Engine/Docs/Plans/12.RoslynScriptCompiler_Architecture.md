# Neverness Roslyn Script Compiler — 架构方案 v4（最终版）

> **日期**: 2026-06-04
> **状态**: ✅ 实施完成（运行时验证待做）

---

## 1. 核心设计

```
Assets/**/*.cs → Parse → CSharpCompilation.Create → Emit(byte[]) → LoadFromBytes → ALC
```

**不做**：SyntaxTreeCache、CompilationCache、FileWatcher
**只做**：Parse → Create → Emit → Load
**缓存**：MetadataReference（启动时创建一次，后续复用）

---

## 2. 关键规则

| 规则 | 说明 |
|------|------|
| PlayMode 不启动编译 | 只加载已编译结果，脚本未就绪则报错 |
| Dirty 用 Version 计数 | 不用 bool，避免并发丢事件 |
| MetadataReference 缓存 | 启动时 `ImmutableArray<MetadataReference>` 创建一次 |
| 不输出 DLL 文件 | Emit → byte[] → LoadFromStream，无磁盘 IO |
| TPA Phase1，RefPack Phase2 | 接口已预留 |
| 保留 DotnetBuildCompiler | ScriptCompilerMode 配置化切换 |
| 保留 .sln/.csproj | IDE 需要 |

---

## 3. 接口设计

```csharp
public enum ScriptCompilerMode { Roslyn, DotnetBuild }

public sealed class ScriptCompilationContext
{
    public required string AssemblyName { get; init; }
    public required IReadOnlyList<string> SourceFiles { get; init; }
    public required IReadOnlyList<MetadataReference> References { get; init; }
    public bool GeneratePdb { get; init; } = true;
    public bool Optimize { get; init; } = false;
}

public sealed class ScriptCompilationResult
{
    public required bool Success { get; init; }
    public byte[]? AssemblyBytes { get; init; }
    public byte[]? PdbBytes { get; init; }
    public ImmutableArray<Diagnostic> Diagnostics { get; init; }
    public TimeSpan Duration { get; init; }
}

public interface IScriptCompiler
{
    ScriptCompilationResult Compile(ScriptCompilationContext context);
}

public interface IReferenceAssemblyProvider
{
    ImmutableArray<MetadataReference> GetReferences();
}
```

---

## 4. Version 计数（替代 bool Dirty）

```csharp
private long _sourceVersion;     // 文件变化时递增
private long _compiledVersion;   // 编译完成时更新

// 文件变化
void OnFileChanged() => Interlocked.Increment(ref _sourceVersion);

// 判断是否需要编译
bool IsDirty => _sourceVersion != _compiledVersion;

// 编译完成
_compiledVersion = targetVersion;
```

---

## 5. Play Mode 流程

```csharp
EnterPlayMode()
{
    GameplayContext.Initialize();

    if (_sourceVersion != _compiledVersion)
    {
        // 脚本有变化但未编译完成 → 报错，不启动编译
        return Error("Scripts have unsaved changes, compile first (F7)");
    }

    if (_cachedAssembly != null)
    {
        // 复用已加载的 Assembly
        RegisterTypesFromAssembly(_cachedAssembly);
    }
    else if (_lastResult?.AssemblyBytes != null)
    {
        // 从内存加载
        loader.LoadFromBytes(_lastResult.AssemblyBytes, _lastResult.PdbBytes);
        RegisterTypesFromAssembly(loader.LoadedAssembly!);
        _cachedAssembly = loader.LoadedAssembly;
    }
    else
    {
        return Error("No compiled assembly available");
    }

    RegisterSystems();
}
```

---

## 6. 后台编译流程

```csharp
// 文件变化 → 递增 version
OnFileChanged() => Interlocked.Increment(ref _sourceVersion);

// 后台编译（不阻塞 Play Mode）
Task.Run(() =>
{
    var targetVersion = _sourceVersion;
    var result = compiler.Compile(context);

    if (result.Success)
    {
        _lastResult = result;
        _compiledVersion = targetVersion;
    }

    // 通知主线程
    CompileFinished?.Invoke(this, result);
});

// 主线程收到通知 → 不自动加载，等 Play Mode 时加载
```

---

## 7. MetadataReference 缓存

```csharp
// Editor 启动时创建一次
var tpaProvider = new TpaReferenceProvider();
_references = tpaProvider.GetReferences();  // ImmutableArray<MetadataReference>

// 后续编译直接传入
var context = new ScriptCompilationContext
{
    References = _references,  // 复用，不重新创建
    ...
};
```

---

## 8. Emit → LoadFromStream（无磁盘 IO）

```csharp
// RoslynScriptCompiler.Emit
var dllStream = new MemoryStream();
var pdbStream = new MemoryStream();
compilation.Emit(dllStream, pdbStream);

return new ScriptCompilationResult
{
    Success = true,
    AssemblyBytes = dllStream.ToArray(),
    PdbBytes = pdbStream.ToArray()
};

// ScriptAssemblyLoader 加载
loader.LoadFromBytes(result.AssemblyBytes, result.PdbBytes);
```

---

## 9. 实施阶段

```
Phase 1: 接口 + RoslynScriptCompiler ✅
  ├── IScriptCompiler / ScriptCompilationContext / ScriptCompilationResult ✅
  ├── IReferenceAssemblyProvider / TpaReferenceProvider ✅
  ├── RoslynScriptCompiler（Parse → Create → Emit byte[]）✅
  └── 验证：编译单个 .cs 文件成功（需运行时验证）

Phase 2: ScriptAssemblyLoader.LoadFromBytes 验证
  └── 已有 LoadFromBytes 方法（ScriptAssemblyLoader.cs:113），无需修改 ✅

Phase 3: ScriptCompileService 接入 ✅
  ├── 使用 IScriptCompiler（构造函数注入）✅
  ├── 后台编译 CompileAsync + 主线程加载 LoadFromLastResult ✅
  ├── Version 计数（_sourceVersion / _compiledVersion）✅
  └── MetadataReference 缓存（TpaReferenceProvider 启动时创建）✅

Phase 4: PlayMode Dirty Check ✅
  ├── EnterPlayMode 检查 IsDirty ✅
  ├── 脚本有变化 → CompileSync（阻塞）✅
  └── 脚本已编译 → LoadFromLastResult ✅

Phase 5: Hot Reload ✅
  ├── DestroyAllImmediate ✅
  ├── CompileSync（同步编译）✅
  └── LoadFromLastResult + Register ✅

Phase 6: DotnetBuildCompiler 回退 ✅
  └── DotnetBuildCompiler 实现 + ScriptCompilerMode 枚举 ✅

Phase 7: RefPack Provider（可选）
Phase 8: SyntaxTreeCache（可选，大概率不需要）
Phase 9: FileWatcher（可选）
```

---

## 10. 文件清单

### 新建

| 文件 | 模块 | 状态 |
|------|------|------|
| `IScriptCompiler.cs` | Runtime.Scripting | [x] |
| `RoslynScriptCompiler.cs` | Runtime.Scripting（重写现有） | [x] |
| `DotnetBuildCompiler.cs` | Runtime.Scripting | [x] |
| `TpaReferenceProvider.cs` | Runtime.Scripting | [x] |
| `IReferenceAssemblyProvider.cs` | Runtime.Scripting | [x] |

### 修改

| 文件 | 变更 | 状态 |
|------|------|------|
| `ScriptCompileService.cs` | 使用 IScriptCompiler，Version 计数，后台编译+主线程加载 | [x] |
| `ScriptEditorModule.cs` | 创建 RoslynScriptCompiler，PlayMode Dirty Check | [x] |

### 保留不改

| 文件 | 说明 |
|------|------|
| `ScriptAssemblyLoader.cs` | 已有 LoadFromBytes |
| `ScriptProjectGenerator.cs` | .sln/.csproj 继续生成 |
| `ScriptBehaviourBridge.cs` | 不变 |
| `ScriptBehaviourScheduler.cs` | 不变 |
| `ScriptRegistry.cs` | 不变 |

---

## 11. 性能预期

| 场景 | dotnet build | Roslyn |
|------|-------------|--------|
| 50 个脚本 | 10-15s | **100-500ms** |
| 100 个脚本 | 10-15s | **300-800ms** |
| 热重载 | 10-15s | **0.5-1.5s** |
| Play Mode（已编译） | 0ms | **<10ms**（LoadFromBytes） |
