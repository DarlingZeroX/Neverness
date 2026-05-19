using Neverness.Managed.Core;
using Neverness.Managed.Engine;

namespace Neverness.Managed.Interop;

/// <summary>打包 Native/Managed API 版本号，供 Entry.GetApiVersion UCO 使用。</summary>
public static class RuntimeVersionInfo
{
	/// <summary>高 16 位 ApiVersion，低 16 位 LayoutVersion。</summary>
	public static int GetPackedApiVersion()
		=> ((int)NNNativeApiConstants.ApiVersion << 16)
		   | ((int)NNNativeEngineApiConstants.LayoutVersion & 0xFFFF);
}
