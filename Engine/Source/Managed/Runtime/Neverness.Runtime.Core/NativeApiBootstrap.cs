using System.Runtime.InteropServices;

namespace Neverness.Managed.Core;

/// <summary>
/// 將 Native 傳入的 <c>NNNativeAPI*</c> 安裝到進程內靜態快取，並提供型別安全的 UTF-8 日誌封裝。
/// Phase 2/3：禁止在此使用 <c>DllImport</c> 調引擎；一律透過函數表間接呼叫。
/// Phase 3：僅接受 <see cref="NNNativeApiConstants.ApiVersion"/>（v2）含 <c>EngineServices</c> 之佈局。
/// </summary>
public static unsafe class NativeApiBootstrap
{
	private static NNNativeApi s_api;
	private static volatile bool s_installed;

	/// <summary>是否已成功安裝且版本相符（供託管側自檢）。</summary>
	public static bool IsInstalled => s_installed;

	/// <summary>
	/// 從 Native 傳入的指標複製一份 API 表至託管靜態儲存（按值複製函數指標，避免依賴指標生命週期）。
	/// </summary>
	/// <param name="nativeApiTable">Native <c>const NNNativeAPI*</c> 轉為 <see cref="nint"/>。</param>
	public static void Install(nint nativeApiTable)
	{
		var p = (NNNativeApi*)nativeApiTable;
		if (p == null)
		{
			// 無效指標：保持未安裝狀態，避免後續誤用未初始化的函數表。
			return;
		}

		if (p->ApiVersion != NNNativeApiConstants.ApiVersion)
		{
			// API 主版本不符：拒絕安裝，防止結構體佈局錯位導致未定義行為。
			return;
		}

		if (p->LogInfo == null)
		{
			// 至少需具備日誌通道，作為「表可用」之最低限度契約（與既有測試一致）。
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
			// 未安裝或 Native 未提供 LogInfo：靜默略過（Bootstrap 前或測試替身情境）。
			return;
		}

		if (utf8Text.IsEmpty)
		{
			// Native 端預期以 NUL 結尾之 C 字串；空訊息仍送單一字節 0 以滿足契約。
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
