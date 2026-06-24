// ============================================================================
// ScriptCompileService.cs - 脚本编译服务
// ============================================================================
// 编排 编译→加载→注册 完整流程。
//
// 设计原则：
// - 编译在后台线程（Roslyn Parse → Create → Emit byte[]）
// - 加载在主线程（ALC LoadFromBytes + RegisterTypes）
// - Version 计数替代 bool Dirty（不丢事件）
// - MetadataReference 启动时缓存，后续复用
// ============================================================================

using System.Collections.Immutable;
using System.Reflection;
using System.Threading;
using Microsoft.CodeAnalysis;
using Neverness.Runtime.VFS;
using Neverness.Gameplay;
using Neverness.Runtime.Scripting;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Script.Private;

/// <summary>
/// 脚本编译服务——编排 编译→加载→注册 完整流程。
/// </summary>
public sealed class ScriptCompileService
{
    // ========================================================================
    // 内部状态
    // ========================================================================

    private readonly IScriptCompiler _compiler;
    private readonly ScriptAssemblyLoader _assemblyLoader = new();
    private readonly IReferenceAssemblyProvider _refProvider;

    // ── Version 计数（替代 bool Dirty）──
    private long _sourceVersion;
    private long _compiledVersion;

    // ── 编译结果缓存 ──
    private ScriptCompilationResult? _lastResult;
    private Assembly? _cachedAssembly;

    // ── MetadataReference 缓存（启动时创建一次）──
    private ImmutableArray<MetadataReference>? _engineRefs;

    // ========================================================================
    // 事件
    // ========================================================================

    /// <summary>编译完成事件（后台线程触发）。</summary>
    public event EventHandler<ScriptCompilationResult>? CompileFinished;

    // ========================================================================
    // 公共属性
    // ========================================================================

    /// <summary>当前加载的程序集。</summary>
    public Assembly? LoadedAssembly => _cachedAssembly ?? _assemblyLoader.LoadedAssembly;

    /// <summary>是否有未编译的变更。</summary>
    public bool IsDirty => Volatile.Read(ref _sourceVersion) != Volatile.Read(ref _compiledVersion);

    /// <summary>最后一次编译结果。</summary>
    public ScriptCompilationResult? LastResult => _lastResult;

    // ========================================================================
    // 构造函数
    // ========================================================================

    public ScriptCompileService(IScriptCompiler compiler, IReferenceAssemblyProvider refProvider)
    {
        _compiler = compiler;
        _refProvider = refProvider;
    }

    // ========================================================================
    // 标记变更
    // ========================================================================

    /// <summary>标记源文件已变更（由 FileWatcher 或 ScriptCompileQueue 调用）。</summary>
    public void MarkSourceChanged()
    {
        Interlocked.Increment(ref _sourceVersion);
    }

    // ========================================================================
    // 编译（后台线程）
    // ========================================================================

    /// <summary>
    /// 后台编译脚本。不阻塞主线程。
    /// 编译完成后触发 CompileFinished 事件。
    /// </summary>
    /// <param name="assembly">程序集定义。</param>
    public void CompileAsync(ScriptAssemblyDefinition assembly)
    {
        var targetVersion = Volatile.Read(ref _sourceVersion);

        // 构建编译上下文
        var context = BuildCompilationContext(assembly);
        if (context == null)
        {
            var errResult = new ScriptCompilationResult
            {
                Success = false,
                Diagnostics = ImmutableArray.Create(
                    Diagnostic.Create(
                        new DiagnosticDescriptor("NN0003", "ContextBuildFailed", "Failed to build compilation context.", "Compilation", DiagnosticSeverity.Error, true),
                        Location.None)),
                Duration = TimeSpan.Zero
            };
            CompileFinished?.Invoke(this, errResult);
            return;
        }

        // 后台编译
        Task.Run(() =>
        {
            var result = _compiler.Compile(context);

            if (result.Success)
            {
                _lastResult = result;
                Volatile.Write(ref _compiledVersion, targetVersion);
                Console.WriteLine($"[ScriptCompileService] Compilation succeeded ({result.Duration.TotalMilliseconds:F0}ms, {result.AssemblyBytes?.Length ?? 0} bytes)");
            }
            else
            {
                Console.WriteLine($"[ScriptCompileService] Compilation failed ({result.ErrorCount} errors)");
            }

            CompileFinished?.Invoke(this, result);
        });
    }

    /// <summary>
    /// 同步编译（用于 Play Mode 前的 Dirty 检查）。
    /// </summary>
    public ScriptCompilationResult CompileSync(ScriptAssemblyDefinition assembly)
    {
        var context = BuildCompilationContext(assembly);
        if (context == null)
        {
            return new ScriptCompilationResult
            {
                Success = false,
                Diagnostics = ImmutableArray.Create(
                    Diagnostic.Create(
                        new DiagnosticDescriptor("NN0003", "ContextBuildFailed", "Failed to build compilation context.", "Compilation", DiagnosticSeverity.Error, true),
                        Location.None)),
                Duration = TimeSpan.Zero
            };
        }

        var result = _compiler.Compile(context);

        if (result.Success)
        {
            _lastResult = result;
            Volatile.Write(ref _compiledVersion, Volatile.Read(ref _sourceVersion));
            Console.WriteLine($"[ScriptCompileService] Sync compilation succeeded ({result.Duration.TotalMilliseconds:F0}ms)");
        }

        CompileFinished?.Invoke(this, result);
        return result;
    }

    // ========================================================================
    // 加载（主线程）
    // ========================================================================

    /// <summary>
    /// 从编译结果加载 Assembly（主线程调用）。
    /// </summary>
    /// <returns>是否成功加载。</returns>
    public bool LoadFromLastResult()
    {
        if (_lastResult?.AssemblyBytes == null)
        {
            Console.WriteLine("[ScriptCompileService] No compiled assembly to load");
            return false;
        }

        _assemblyLoader.LoadFromBytes(_lastResult.AssemblyBytes, _lastResult.PdbBytes);
        RegisterTypesFromAssembly(_assemblyLoader.LoadedAssembly!);
        _cachedAssembly = _assemblyLoader.LoadedAssembly;
        return true;
    }

    // ========================================================================
    // 热重载
    // ========================================================================

    /// <summary>
    /// 热重载：立即销毁 Behaviour → 编译 → 加载 → 注册。
    /// </summary>
    public ScriptCompilationResult HotReload(ScriptAssemblyDefinition assembly)
    {
        // 1. 立即销毁所有 Behaviour（OnDestroy 在旧 ALC 中执行）
        var scheduler = GameplayContext.Current?.BehaviourScheduler;
        scheduler?.DestroyAllImmediate();

        // 2. 清除缓存
        _cachedAssembly = null;
        _lastResult = null;

        // 3. 同步编译
        var result = CompileSync(assembly);

        if (result.Success)
        {
            // 4. 主线程加载
            LoadFromLastResult();
        }

        return result;
    }

    // ========================================================================
    // 内部方法
    // ========================================================================

    /// <summary>构建编译上下文。</summary>
    private ScriptCompilationContext? BuildCompilationContext(ScriptAssemblyDefinition assembly)
    {
        // 1. 扫描源文件
        var assetsDir = VFSService.GetAbsolutePath(ProjectPaths.Assets.FullPath);
        if (string.IsNullOrEmpty(assetsDir))
        {
            Console.WriteLine("[ScriptCompileService] Assets directory not resolvable");
            return null;
        }

        var sourceFiles = Directory.GetFiles(assetsDir, "*.cs", SearchOption.AllDirectories)
            .Where(f => !f.Contains(Path.Combine("obj", "")) &&
                        !f.Contains(Path.Combine("bin", "")) &&
                        !f.Contains(Path.Combine("Build", "")))
            .ToArray();

        if (sourceFiles.Length == 0)
        {
            Console.WriteLine("[ScriptCompileService] No .cs files found");
            return null;
        }

        // 2. 获取 MetadataReference（缓存引擎引用）
        if (!_engineRefs.HasValue)
        {
            _engineRefs = ResolveEngineReferences(assembly);
        }

        var allRefs = _refProvider.GetReferences().AddRange(_engineRefs.Value);

        Console.WriteLine($"[ScriptCompileService] Building context: {sourceFiles.Length} files, {allRefs.Length} references");

        return new ScriptCompilationContext
        {
            AssemblyName = assembly.Name,
            SourceFiles = sourceFiles,
            References = allRefs,
            GeneratePdb = true,
            Optimize = false,
            TargetFramework = assembly.TargetFramework
        };
    }

    /// <summary>解析引擎程序集引用。</summary>
    private ImmutableArray<MetadataReference> ResolveEngineReferences(ScriptAssemblyDefinition assembly)
    {
        var builder = ImmutableArray.CreateBuilder<MetadataReference>();

        var projectRoot = VFSService.GetAbsolutePath("/");
        if (string.IsNullOrEmpty(projectRoot))
            return builder.ToImmutable();

        // 从 DefaultEngineAssemblyResolver 获取引擎 DLL 路径
        var engineBinariesDir = GetEngineBinariesDir();
        foreach (var name in assembly.References)
        {
            var dllPath = Path.Combine(engineBinariesDir, $"{name}.dll");
            if (File.Exists(dllPath))
            {
                try
                {
                    builder.Add(MetadataReference.CreateFromFile(dllPath));
                    Console.WriteLine($"[ScriptCompileService] Engine ref: {name} → {dllPath}");
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[ScriptCompileService] Failed to load engine ref {name}: {ex.Message}");
                }
            }
            else
            {
                Console.WriteLine($"[ScriptCompileService] Engine DLL not found: {dllPath}");
            }
        }

        return builder.ToImmutable();
    }

    /// <summary>获取引擎 DLL 目录。</summary>
    private static string GetEngineBinariesDir()
    {
        var engineRoot = VFSService.GetAbsolutePath("/");
        if (!string.IsNullOrEmpty(engineRoot))
        {
            var candidates = new[]
            {
                Path.Combine(engineRoot, "Engine", "Build", "bin", "Debug"),
                Path.Combine(engineRoot, "Engine", "Build", "bin", "Release"),
                Path.Combine(engineRoot, "Build", "bin", "Debug"),
                Path.Combine(engineRoot, "Build", "bin", "Release"),
            };

            foreach (var candidate in candidates)
            {
                if (Directory.Exists(candidate))
                    return candidate;
            }
        }

        return AppContext.BaseDirectory;
    }

    /// <summary>扫描程序集，注册 EntityBehaviour 子类到 ScriptRegistry。</summary>
    private void RegisterTypesFromAssembly(Assembly assembly)
    {
        var registry = GameplayContext.Current?.ScriptRegistry;
        if (registry == null)
        {
            Console.WriteLine("[ScriptCompileService] ScriptRegistry not available, skipping type registration");
            return;
        }

        int count = 0;
        foreach (var type in assembly.GetTypes())
        {
            if (type.IsAbstract || type.IsInterface)
                continue;
            if (!typeof(EntityBehaviour).IsAssignableFrom(type))
                continue;

            registry.Register(type);
            count++;
        }

        Console.WriteLine($"[ScriptCompileService] Registered {count} script types from {assembly.GetName().Name}");
    }
}
