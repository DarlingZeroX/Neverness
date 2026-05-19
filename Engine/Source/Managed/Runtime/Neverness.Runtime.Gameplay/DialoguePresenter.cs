using System.Text;
using Neverness.Managed.Engine;
using Neverness.Managed.Interop;

namespace Neverness.Managed.Gameplay;

/// <summary>
/// 對白呈現：經 Engine Service <c>NNUIApi.SetDialogueText</c> 將文字送至 UI 層（Stub 或 Runtime 實作）。
/// <para>未安裝 ABI 時 <see cref="PresentLine"/> 靜默返回 false，不拋出例外（供 <see cref="PresentDialogueSequenceStep"/> 與測試區分）。</para>
/// </summary>
public static unsafe class DialoguePresenter
{
	/// <summary>
	/// 在指定 UI 元素上顯示對白文字（內部轉為 UTF-8 位元組後呼叫 Native）。
	/// </summary>
	/// <param name="elementHandle">Native UI 元素控制代碼；0 表示由引擎決定預設元素（若支援）。</param>
	/// <param name="text">對白內容（UTF-16）；會以 <see cref="Encoding.UTF8"/> 編碼。</param>
	/// <returns>若 ABI 已安裝且 <c>SetDialogueText</c> 非空並已呼叫則為 true；否則 false。</returns>
	public static bool PresentLine(ulong elementHandle, ReadOnlySpan<char> text)
	{
		// 未經 Bootstrap 安裝引擎表時，不拋例外、僅表示「無法呈現」（與序列步驟之寬鬆語意對齊）。
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return false;
		}

		ref readonly var ui = ref EngineNativeApiBootstrap.EngineApi.UI;
		// 當前 ABI 子表未提供 SetDialogueText 指標時同樣視為不可用。
		if (ui.SetDialogueText == null)
		{
			return false;
		}

		// 將 UTF-16 文本編碼為 UTF-8 位元組陣列後以指標傳入 Native（Stub 或實作自行解讀長度語意）。
		var utf8 = Encoding.UTF8.GetBytes(text.ToString());
		fixed (byte* pText = utf8)
		{
			ui.SetDialogueText(elementHandle, pText);
		}

		return true;
	}

	/// <summary>以 managed 字串顯示對白（委派至 <see cref="PresentLine(ulong, ReadOnlySpan{char})"/>）。</summary>
	public static bool PresentLine(ulong elementHandle, string text)
	{
		ArgumentNullException.ThrowIfNull(text);
		return PresentLine(elementHandle, text.AsSpan());
	}
}
