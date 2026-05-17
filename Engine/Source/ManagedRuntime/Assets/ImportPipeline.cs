using System.Text;
using VisionGal.Managed.Engine;

namespace VisionGal.Managed.Assets;

/// <summary>
/// 資產匯入管線：虛擬路徑 → Native <c>importAsset</c> → 登記至 <see cref="AssetDatabase"/>。
/// </summary>
public static class ImportPipeline
{
	/// <summary>
	/// 匯入指定虛擬路徑之資產。
	/// 優先經 Native <c>AssetRegistry.importAsset</c>；若回傳零 GUID 則使用 <see cref="GUID.FromDeterministicPath"/> 穩定合成 ID（不再對路徑做 <see cref="GUID.Parse"/>）。
	/// </summary>
	/// <param name="virtualPath">NUL 結尾語意之虛擬路徑字串。</param>
	/// <returns>已登記之 GUID。</returns>
	public static GUID Import(string virtualPath)
	{
		ArgumentException.ThrowIfNullOrWhiteSpace(virtualPath);

		var guid = ImportNative(virtualPath);
		if (guid.IsZero)
		{
			guid = GUID.FromDeterministicPath(virtualPath);
		}

		AssetDatabase.Register(virtualPath, guid);
		return guid;
	}

	/// <summary>僅呼叫 Native 匯入，不寫入 <see cref="AssetDatabase"/>。</summary>
	private static unsafe GUID ImportNative(string virtualPath)
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

		var bytes = Encoding.UTF8.GetBytes(virtualPath);
		fixed (byte* p = bytes)
		{
			return GUID.FromNative(fn(p));
		}
	}
}
