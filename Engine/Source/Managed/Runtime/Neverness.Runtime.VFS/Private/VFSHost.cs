// Neverness.Runtime.VFS — NNVfsAPI 托管门面；禁止在产品代码中散落 delegate*。

using System.Runtime.InteropServices;
using System.Text;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Runtime.VFS;

/// <summary>
/// 封装 <c>NNVfsAPI</c> 函数指针；负责 UTF-8 路径编组与 Native 缓冲区释放。
/// </summary>
internal static unsafe class VFSHost
{
	/// <summary>VFS 子表是否已安装且可用。</summary>
	public static bool IsAvailable =>
		EngineNativeApiBootstrap.IsInstalled &&
		EngineNativeApiBootstrap.EngineApi.Vfs.ReadText != null;

	public static string? ReadText(string path)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(path);
		if (!TryGetVfsApi(out var api) || api.ReadText == null || api.FreeBuffer == null)
		{
			return null;
		}

		var utf8Path = Encoding.UTF8.GetBytes(path + '\0');
		fixed (byte* pPath = utf8Path)
		{
			byte* nativeText = null;
			if (api.ReadText(pPath, &nativeText) == 0 || nativeText == null)
			{
				return null;
			}

			try
			{
				return Marshal.PtrToStringUTF8((nint)nativeText);
			}
			finally
			{
				api.FreeBuffer(nativeText);
			}
		}
	}

	public static bool WriteText(string path, string text)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(path);
		ArgumentNullException.ThrowIfNull(text);
		if (!TryGetVfsApi(out var api) || api.WriteText == null)
		{
			return false;
		}

		var utf8Path = Encoding.UTF8.GetBytes(path + '\0');
		var utf8Text = Encoding.UTF8.GetBytes(text + '\0');
		fixed (byte* pPath = utf8Path)
		fixed (byte* pText = utf8Text)
		{
			return api.WriteText(pPath, pText) != 0;
		}
	}

	public static byte[]? ReadBytes(string path)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(path);
		if (!TryGetVfsApi(out var api) || api.ReadBytes == null || api.FreeBuffer == null)
		{
			return null;
		}

		var utf8Path = Encoding.UTF8.GetBytes(path + '\0');
		fixed (byte* pPath = utf8Path)
		{
			byte* nativeData = null;
			uint size = 0;
			if (api.ReadBytes(pPath, &nativeData, &size) == 0)
			{
				return null;
			}

			try
			{
				if (nativeData == null || size == 0)
				{
					return Array.Empty<byte>();
				}

				var result = new byte[size];
				Marshal.Copy((nint)nativeData, result, 0, (int)size);
				return result;
			}
			finally
			{
				if (nativeData != null)
				{
					api.FreeBuffer(nativeData);
				}
			}
		}
	}

	/// <summary>
	/// 将磁盘绝对路径转换为相对 <paramref name="relativePath"/> 的 VFS 路径。
	/// </summary>
	public static string? GetRelativePath(string relativePath, string absolutePath)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(relativePath);
		ArgumentException.ThrowIfNullOrWhiteSpace(absolutePath);
		if (!TryGetVfsApi(out var api) || api.GetRelativePath == null || api.FreeBuffer == null)
		{
			return null;
		}

		var utf8Relative = Encoding.UTF8.GetBytes(relativePath + '\0');
		var utf8Absolute = Encoding.UTF8.GetBytes(absolutePath + '\0');
		fixed (byte* pRelative = utf8Relative)
		fixed (byte* pAbsolute = utf8Absolute)
		{
			byte* nativePath = null;
			if (api.GetRelativePath(pRelative, pAbsolute, &nativePath) == 0 || nativePath == null)
			{
				return null;
			}

			try
			{
				return Marshal.PtrToStringUTF8((nint)nativePath);
			}
			finally
			{
				api.FreeBuffer(nativePath);
			}
		}
	}

	/// <summary>
	/// 将 VFS 虚拟路径解析为绝对路径（<c>VFSService::GetAbsolutePath</c>）。
	/// </summary>
	public static string? GetAbsolutePath(string relativePath)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(relativePath);
		if (!TryGetVfsApi(out var api) || api.GetAbsolutePath == null || api.FreeBuffer == null)
		{
			return null;
		}

		var utf8Path = Encoding.UTF8.GetBytes(relativePath + '\0');
		fixed (byte* pPath = utf8Path)
		{
			byte* nativePath = null;
			if (api.GetAbsolutePath(pPath, &nativePath) == 0 || nativePath == null)
			{
				return null;
			}

			try
			{
				return Marshal.PtrToStringUTF8((nint)nativePath);
			}
			finally
			{
				api.FreeBuffer(nativePath);
			}
		}
	}

	/// <summary>
	/// 刷新指定 VFS 路径下单一 Native 文件系统的文件列表缓存。
	/// </summary>
	public static bool RebuildNativeFileSystemFiles(string path)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(path);
		if (!TryGetVfsApi(out var api) || api.RebuildNativeFileSystemFiles == null)
		{
			return false;
		}

		var utf8Path = Encoding.UTF8.GetBytes(path + '\0');
		fixed (byte* pPath = utf8Path)
		{
			return api.RebuildNativeFileSystemFiles(pPath) != 0;
		}
	}

	/// <summary>将二进制缓冲区写入 VFS 路径（覆盖写）。</summary>
	public static bool WriteBuffer(string path, byte[] buffer)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(path);
		ArgumentNullException.ThrowIfNull(buffer);
		if (!TryGetVfsApi(out var api) || api.WriteBufferToFile == null)
		{
			return false;
		}

		var utf8Path = Encoding.UTF8.GetBytes(path + '\0');
		fixed (byte* pPath = utf8Path)
		fixed (byte* pData = buffer)
		{
			return api.WriteBufferToFile(pPath, pData, (ulong)buffer.Length) != 0;
		}
	}

	/// <summary>创建并挂载文件系统，返回 handle（0 失败）。</summary>
	public static ulong AddFileSystem(string alias, NNVfsFileSystemType type, string? path)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(alias);
		if (!TryGetVfsApi(out var api) || api.AddFileSystem == null)
		{
			return 0;
		}

		var utf8Alias = Encoding.UTF8.GetBytes(alias + '\0');
		var utf8Path = Encoding.UTF8.GetBytes((path ?? string.Empty) + '\0');
		fixed (byte* pAlias = utf8Alias)
		fixed (byte* pPath = utf8Path)
		{
			return api.AddFileSystem(pAlias, type, pPath);
		}
	}

	/// <summary>根据 handle 精确移除文件系统。</summary>
	public static bool RemoveFileSystem(ulong handle)
	{
		if (!TryGetVfsApi(out var api) || api.RemoveFileSystem == null)
		{
			return false;
		}
		return api.RemoveFileSystem(handle) != 0;
	}

	/// <summary>查询 handle 是否仍在 VFS 中。</summary>
	public static bool HasFileSystem(ulong handle)
	{
		if (!TryGetVfsApi(out var api) || api.HasFileSystem == null)
		{
			return false;
		}
		return api.HasFileSystem(handle) != 0;
	}

	/// <summary>移除 alias 下全部文件系统。</summary>
	public static void UnregisterAlias(string alias)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(alias);
		if (!TryGetVfsApi(out var api) || api.UnregisterAlias == null)
		{
			return;
		}

		var utf8Alias = Encoding.UTF8.GetBytes(alias + '\0');
		fixed (byte* pAlias = utf8Alias)
		{
			api.UnregisterAlias(pAlias);
		}
	}

	/// <summary>查询 alias 是否已注册。</summary>
	public static bool IsAliasRegistered(string alias)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(alias);
		if (!TryGetVfsApi(out var api) || api.IsAliasRegistered == null)
		{
			return false;
		}

		var utf8Alias = Encoding.UTF8.GetBytes(alias + '\0');
		fixed (byte* pAlias = utf8Alias)
		{
			return api.IsAliasRegistered(pAlias) != 0;
		}
	}

	private static bool TryGetVfsApi(out NNVfsApi api)
	{
		api = default;
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return false;
		}

		api = EngineNativeApiBootstrap.EngineApi.Vfs;
		return true;
	}
}
