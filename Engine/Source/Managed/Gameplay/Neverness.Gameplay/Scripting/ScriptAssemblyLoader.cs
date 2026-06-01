// ============================================================================
// ScriptAssemblyLoader.cs - 脚本程序集加载器
// ============================================================================
// 加载编译后的脚本程序集，支持热重载。
// 仅在 Editor JIT 模式下使用。
// ============================================================================

using System.Reflection;
using System.Runtime.Loader;

namespace Neverness.Gameplay;

/// <summary>
/// 脚本程序集加载器：加载编译后的脚本程序集。
/// </summary>
/// <remarks>
/// ⚠️ 仅在 Editor JIT 模式下使用。
/// Release 模式使用 Source Generator 静态注册，不使用此加载器。
///
/// 使用 ALC（AssemblyLoadContext）隔离加载，支持热重载。
/// </remarks>
public sealed class ScriptAssemblyLoader : IDisposable
{
    // ========================================================================
    // 内部类型
    // ========================================================================

    /// <summary>
    /// 可收集的 AssemblyLoadContext，用于热重载。
    /// </summary>
    private sealed class CollectibleAssemblyLoadContext : AssemblyLoadContext
    {
        public CollectibleAssemblyLoadContext(string name)
            : base(name, isCollectible: true)
        {
        }

        protected override Assembly? Load(AssemblyName assemblyName)
        {
            // 不从其他上下文加载，依赖默认解析
            return null;
        }
    }

    // ========================================================================
    // 内部状态
    // ========================================================================

    /// <summary>当前 ALC。</summary>
    private CollectibleAssemblyLoadContext? _currentContext;

    /// <summary>当前加载的程序集。</summary>
    private Assembly? _loadedAssembly;

    /// <summary>当前加载的程序集路径。</summary>
    private string? _loadedPath;

    /// <summary>是否已释放。</summary>
    private bool _isDisposed;

    // ========================================================================
    // 公共属性
    // ========================================================================

    /// <summary>当前加载的程序集。</summary>
    public Assembly? LoadedAssembly => _loadedAssembly;

    /// <summary>当前加载的程序集路径。</summary>
    public string? LoadedPath => _loadedPath;

    /// <summary>是否已加载程序集。</summary>
    public bool IsLoaded => _loadedAssembly is not null;

    // ========================================================================
    // 公共方法
    // ========================================================================

    /// <summary>
    /// 加载脚本程序集。
    /// </summary>
    /// <param name="assemblyPath">程序集路径。</param>
    /// <returns>加载的程序集。</returns>
    public Assembly Load(string assemblyPath)
    {
        ObjectDisposedException.ThrowIf(_isDisposed, this);
        ArgumentException.ThrowIfNullOrWhiteSpace(assemblyPath);

        var fullPath = Path.GetFullPath(assemblyPath);
        if (!File.Exists(fullPath))
        {
            throw new FileNotFoundException($"Assembly not found: {fullPath}");
        }

        // 卸载旧的 ALC
        UnloadCurrent();

        // 创建新的 ALC
        _currentContext = new CollectibleAssemblyLoadContext($"Neverness.UserScripts.{Guid.NewGuid():N}");

        // 加载程序集
        _loadedAssembly = _currentContext.LoadFromAssemblyPath(fullPath);
        _loadedPath = fullPath;

        return _loadedAssembly;
    }

    /// <summary>
    /// 从字节数组加载脚本程序集。
    /// </summary>
    /// <param name="assemblyBytes">程序集字节数组。</param>
    /// <param name="symbolBytes">符号字节数组（可选）。</param>
    /// <returns>加载的程序集。</returns>
    public Assembly LoadFromBytes(byte[] assemblyBytes, byte[]? symbolBytes = null)
    {
        ObjectDisposedException.ThrowIf(_isDisposed, this);
        ArgumentNullException.ThrowIfNull(assemblyBytes);

        // 卸载旧的 ALC
        UnloadCurrent();

        // 创建新的 ALC
        _currentContext = new CollectibleAssemblyLoadContext($"Neverness.UserScripts.{Guid.NewGuid():N}");

        // 加载程序集
        using var assemblyStream = new MemoryStream(assemblyBytes);
        using var symbolStream = symbolBytes is not null ? new MemoryStream(symbolBytes) : null;

        _loadedAssembly = _currentContext.LoadFromStream(assemblyStream, symbolStream);
        _loadedPath = null;

        return _loadedAssembly;
    }

    /// <summary>
    /// 卸载当前 ALC。
    /// </summary>
    public void UnloadCurrent()
    {
        if (_currentContext is null)
        {
            return;
        }

        // 卸载 ALC
        _currentContext.Unload();
        _currentContext = null;
        _loadedAssembly = null;
        _loadedPath = null;
    }

    /// <summary>
    /// 重新加载脚本程序集（热重载）。
    /// </summary>
    /// <param name="assemblyPath">程序集路径。</param>
    /// <returns>加载的程序集。</returns>
    public Assembly Reload(string assemblyPath)
    {
        return Load(assemblyPath);
    }

    // ========================================================================
    // IDisposable
    // ========================================================================

    /// <inheritdoc/>
    public void Dispose()
    {
        if (_isDisposed)
        {
            return;
        }

        UnloadCurrent();
        _isDisposed = true;
    }
}
