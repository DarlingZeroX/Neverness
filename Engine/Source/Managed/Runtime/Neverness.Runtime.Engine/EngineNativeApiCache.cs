namespace Neverness.Runtime.Engine;

/// <summary>
/// 已安装的 <see cref="NNNativeEngineApi"/> 进程内缓存；由 <c>Neverness.Runtime.Interop</c> 在安装时写入。
/// </summary>
public static unsafe class EngineNativeApiCache
{
	private static NNNativeEngineApi s_engineApi;
	private static volatile bool s_installed;

	/// <summary>是否已成功缓存引擎服务表。</summary>
	public static bool IsInstalled => s_installed;

	/// <summary>已安装引擎服务表；未安装时为零结构。</summary>
	public static ref readonly NNNativeEngineApi EngineApi => ref s_engineApi;

	/// <summary>由 Interop 引导路径调用；测试可经 <see cref="ResetForTesting"/> 清空。</summary>
	public static void Install(in NNNativeEngineApi api)
	{
		s_engineApi = api;
		s_installed = true;
	}

	/// <summary>测试重置。</summary>
	public static void ResetForTesting()
	{
		s_engineApi = default;
		s_installed = false;
	}
}
