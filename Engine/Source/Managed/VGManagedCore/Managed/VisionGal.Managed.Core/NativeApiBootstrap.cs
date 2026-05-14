using System.Runtime.InteropServices;

namespace VisionGal.Managed.Core;

/// <summary>
/// 將 Native 傳入的 <c>VGNativeAPI*</c> 安裝到進程內靜態快取，並提供型別安全的 UTF-8 日誌封裝。
/// Phase 2/3：禁止在此使用 <c>DllImport</c> 調引擎；一律透過函數表間接呼叫。
/// Phase 3：僅接受 <see cref="VGNativeApiConstants.ApiVersion"/>（v2）含 <c>EngineServices</c> 之佈局。
/// </summary>
public static unsafe class NativeApiBootstrap
{
	private static VGNativeApi s_api;
	private static volatile bool s_installed;

	/// <summary>是否已成功安裝且版本相符（供託管側自檢）。</summary>
	public static bool IsInstalled => s_installed;

	/// <summary>
	/// 從 Native 傳入的指標複製一份 API 表至託管靜態儲存（按值複製函數指標，避免依賴指標生命週期）。
	/// </summary>
	/// <param name="nativeApiTable">Native <c>const VGNativeAPI*</c> 轉為 <see cref="nint"/>。</param>
	public static void Install(nint nativeApiTable)
	{
		var p = (VGNativeApi*)nativeApiTable;
		if (p == null)
		{
			return;
		}

		if (p->ApiVersion != VGNativeApiConstants.ApiVersion)
		{
			return;
		}

		if (p->LogInfo == null)
		{
			return;
		}

		s_api = *p;
		s_installed = true;
	}

	/// <summary>向 Native 日誌通道寫入一段 UTF-8 文字（無需 NUL 結尾 span，內部暫時補零）。</summary>
	public static void LogInfoUtf8(ReadOnlySpan<byte> utf8Text)
	{
		if (!s_installed || s_api.LogInfo == null)
		{
			return;
		}

		if (utf8Text.IsEmpty)
		{
			ReadOnlySpan<byte> empty = [(byte)'\0'];
			fixed (byte* pb = empty)
			{
				s_api.LogInfo(pb);
			}

			return;
		}

		// 棧上緩衝：Phase 2 僅用於短 smoke 訊息；後續可改為 Native 側顯式長度參數或共享緩衝契約。
		Span<byte> buf = stackalloc byte[utf8Text.Length + 1];
		utf8Text.CopyTo(buf);
		buf[^1] = 0;
		fixed (byte* pb = buf)
		{
			s_api.LogInfo(pb);
		}
	}
}
