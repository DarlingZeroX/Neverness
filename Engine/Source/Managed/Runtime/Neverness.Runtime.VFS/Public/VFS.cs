// Neverness.Runtime.VFS — 业务层 VFS 门面（经函数表调用 Native VFSService）。

namespace Neverness.Runtime.VFS.Public;

/// <summary>
/// 虚拟文件系统托管 API（Phase 1：ReadText / WriteText / ReadBytes）。
/// 调用前须完成 Native Bootstrap 且 VFS 已由宿主挂载（例如 Editor <c>ApplicationHost.Initialize</c> 路径）。
/// </summary>
public static class VFS
{
	/// <summary>VFS 子表是否可用。</summary>
	public static bool IsAvailable => VFSHost.IsAvailable;

	/// <summary>
	/// 读取 VFS 路径下的 UTF-8 文本全文；失败返回 <see langword="null"/>。
	/// </summary>
	/// <param name="path">VFS 虚拟路径（将编码为 UTF-8）。</param>
	public static string? ReadText(string path) => VFSHost.ReadText(path);

	/// <summary>
	/// 将 UTF-8 文本写入 VFS 路径（覆盖写）。
	/// </summary>
	public static bool WriteText(string path, string text) => VFSHost.WriteText(path, text);

	/// <summary>
	/// 读取 VFS 路径下的二进制全文；失败返回 <see langword="null"/>；空文件返回零长度数组。
	/// </summary>
    public static byte[]? ReadBytes(string path) => VFSHost.ReadBytes(path);

	/// <summary>
	/// 将 VFS 虚拟路径解析为绝对路径（<c>VFSService::GetAbsolutePath</c>）。
	/// </summary>
	public static string? GetAbsolutePath(string relativePath) =>
		VFSHost.GetAbsolutePath(relativePath);

	/// <summary>
	/// 计算 VFS 相对路径（<c>VFSService::GetRelativePathVFS</c>）。
	/// </summary>
	public static string? GetRelativePath(string relativePath, string absolutePath) =>
		VFSHost.GetRelativePath(relativePath, absolutePath);

	/// <summary>
	/// 将二进制缓冲区写入 VFS 路径（覆盖写）。
	/// </summary>
	public static bool WriteBuffer(string path, byte[] buffer) =>
		VFSHost.WriteBuffer(path, buffer);

	/// <summary>
	/// 刷新指定路径下 Native 文件系统文件列表（<c>VFSService::RebuildNativeFileSystemFiles</c>）。
	/// </summary>
	public static bool RebuildNativeFileSystemFiles(string path) =>
		VFSHost.RebuildNativeFileSystemFiles(path);
}
