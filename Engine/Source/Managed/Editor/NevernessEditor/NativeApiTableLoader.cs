using System.Runtime.InteropServices;

namespace NevernessEditor;

/// <summary>
/// 从 <c>NevernessRuntime-Managed.dll</c> 解析 <c>NNNativeApi_GetDefaultTable</c> 导出。
/// 仅用于 Editor 可执行文件；<see cref="Neverness.Runtime.Interop"/> 仍禁止 DllImport。
/// </summary>
internal static class NativeApiTableLoader
{
	private const string ManagedDllFileName = "NevernessRuntime-EngineServices.dll";
	private const string ExportName = "NNNativeApi_GetDefaultTable";
	private const string EnvDllPath = "NEVERNESS_NATIVE_MANAGED_DLL";

	private static nint s_cachedTable;
	private static IntPtr s_libraryHandle;

	/// <summary>
	/// 尝试解析 Native API 表指针；成功时写入 <paramref name="nativeApiTable"/> 并返回 true。
	/// </summary>
	/// <param name="nativeApiTable">输出：<c>NNNativeAPI*</c> 指针。</param>
	/// <param name="errorMessage">失败时的中文说明（含已搜索路径）。</param>
	public static bool TryResolve(out nint nativeApiTable, out string? errorMessage)
	{
		if (s_cachedTable != 0)
		{
			nativeApiTable = s_cachedTable;
			errorMessage = null;
			return true;
		}

		var searched = new List<string>();
		foreach (var candidate in EnumerateCandidatePaths())
		{
			searched.Add(candidate);
			if (!File.Exists(candidate))
			{
				continue;
			}

			if (TryLoadFromPath(candidate, out nativeApiTable, out var loadError))
			{
				s_cachedTable = nativeApiTable;
				errorMessage = null;
				return true;
			}

			if (loadError != null)
			{
				errorMessage =
					$"已找到 {ManagedDllFileName} 但加载失败：{loadError}\n" +
					$"路径：{candidate}\n" +
					"请确认 SDL3.dll 与 Managed.dll 位于同一目录，或已加入 PATH。";
				nativeApiTable = 0;
				return false;
			}
		}

		errorMessage =
			$"未找到 {ManagedDllFileName} 或缺少导出符号 {ExportName}。\n" +
			"请先 CMake 构建 NevernessRuntime-Managed，并确保 PostBuild 已复制 DLL。\n" +
			"已搜索路径：\n" + string.Join('\n', searched.Select(static p => "  - " + p));
		nativeApiTable = 0;
		return false;
	}

	/// <summary>按优先级枚举候选 DLL 绝对路径。</summary>
	private static IEnumerable<string> EnumerateCandidatePaths()
	{
		var envPath = Environment.GetEnvironmentVariable(EnvDllPath);
		if (!string.IsNullOrWhiteSpace(envPath))
		{
			yield return Path.GetFullPath(envPath);
		}

		var baseDir = AppContext.BaseDirectory;
		yield return Path.Combine(baseDir, ManagedDllFileName);

		// dotnet build 输出与 CMake 产物可能同在 $(SolutionDir)Build/bin/$(Configuration)
		var config = Environment.GetEnvironmentVariable("Configuration") ?? "Debug";
		var dir = new DirectoryInfo(baseDir);
		for (var i = 0; i < 8 && dir != null; i++, dir = dir.Parent)
		{
			var fromBuild = Path.Combine(dir.FullName, "Build", "bin", config, ManagedDllFileName);
			yield return fromBuild;
		}
	}

	/// <summary>从指定路径加载 DLL 并调用导出函数。</summary>
	private static bool TryLoadFromPath(string dllPath, out nint nativeApiTable, out string? loadError)
	{
		nativeApiTable = 0;
		loadError = null;

		try
		{
			s_libraryHandle = NativeLibrary.Load(dllPath);
			if (!NativeLibrary.TryGetExport(s_libraryHandle, ExportName, out var exportPtr))
			{
				loadError = $"未找到导出 {ExportName}。";
				return false;
			}

			var getTable = Marshal.GetDelegateForFunctionPointer<GetDefaultTableDelegate>(exportPtr);
			nativeApiTable = getTable();
			if (nativeApiTable == 0)
			{
				loadError = $"{ExportName} 返回空指针。";
				return false;
			}

			return true;
		}
		catch (DllNotFoundException ex)
		{
			loadError = $"依赖项缺失：{ex.Message}";
			return false;
		}
		catch (Exception ex)
		{
			loadError = ex.Message;
			return false;
		}
	}

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	private delegate nint GetDefaultTableDelegate();
}
