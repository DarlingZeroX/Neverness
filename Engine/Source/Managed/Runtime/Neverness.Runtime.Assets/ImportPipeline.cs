using System.Text;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Runtime.Assets;

/// <summary>
/// 資產匯入管線：虛擬路徑 → Native <c>importAsset</c> → 登記至 <see cref="AssetDatabase"/>。
/// </summary>
public static class ImportPipeline
{
	/// <summary>
	/// 匯入指定虛擬路徑之資產。
	/// 優先經 Native <c>AssetRegistry.importAsset</c>；若回傳零 GUID 則使用 <see cref="GUID.FromDeterministicPath"/> 穩定合成 ID。
	/// </summary>
	/// <param name="path">虛擬路徑。</param>
	/// <returns>已登記之 GUID。</returns>
	public static GUID Import(NVirtualPath path)
	{
		if (path.IsEmpty)
		{
			return GUID.Zero;
		}

		var guid = ImportNative(path);
		if (guid.IsZero)
		{
			guid = GUID.FromDeterministicPath(path.FullPath);
		}

		AssetDatabase.Register(path, guid);
		return guid;
	}

	/// <summary>僅呼叫 Native 匯入，不寫入 <see cref="AssetDatabase"/>。</summary>
	private static unsafe GUID ImportNative(NVirtualPath path)
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return GUID.Zero;
		}

		var fn = EngineNativeApiBootstrap.EngineApi.AssetRegistry.ImportAsset;
		if (fn == null)
		{
			return GUID.Zero;
		}

		var bytes = Encoding.UTF8.GetBytes(path.FullPath);
		fixed (byte* p = bytes)
		{
			return GUID.FromNative(fn(p));
		}
	}
}
