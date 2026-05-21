using System.Reflection;

namespace Neverness.Runtime.Scripting;

/// <summary>
/// 熱重載協調器：卸載可收集 ALC 並以新程序集路徑重新載入。
/// </summary>
public sealed class HotReloadCoordinator
{
	private ManagedAssemblyLoadContextHost? _currentHost;
	private string? _loadedPath;

	/// <summary>目前載入之程序集路徑。</summary>
	public string? LoadedPath => _loadedPath;

	/// <summary>載入或熱重載指定路徑之程序集。</summary>
	public Assembly Reload(string assemblyPath)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(assemblyPath);
		UnloadCurrent();
		_currentHost = new ManagedAssemblyLoadContextHost($"Neverness.Scripting.{Guid.NewGuid():N}");
		var asm = _currentHost.LoadFromPath(assemblyPath);
		_loadedPath = assemblyPath;
		return asm;
	}

	/// <summary>卸載目前 ALC（需無外部強引用方可 GC）。</summary>
	public void UnloadCurrent()
	{
		if (_currentHost == null)
		{
			return;
		}

		_currentHost.Unload();
		_currentHost = null;
		_loadedPath = null;
	}
}
