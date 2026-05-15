using System.Runtime.InteropServices;
using System.Text;
using VisionGal.Managed.Engine;

namespace VisionGal.Managed.Gameplay;

/// <summary>
/// 對白呈現：經 Engine Service <c>VGUIApi.SetDialogueText</c> 將文字送至 UI 層（Stub 或 Runtime 實作）。
/// <para>未安裝 ABI 時 <see cref="PresentLine"/> 靜默返回，不拋出例外。</para>
/// </summary>
public static unsafe class DialoguePresenter
{
	/// <summary>
	/// 在指定 UI 元素上顯示對白文字（UTF-8）。
	/// </summary>
	/// <param name="elementHandle">Native UI 元素控制代碼；0 表示使用預設元素（仍由 Native 決定是否接受）。</param>
	/// <param name="text">對白內容。</param>
	/// <returns>是否已呼叫 Native（ABI 已安裝且函數指標非空）。</returns>
	public static bool PresentLine(ulong elementHandle, ReadOnlySpan<char> text)
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return false;
		}

		ref readonly var ui = ref EngineNativeApiBootstrap.EngineApi.UI;
		if (ui.SetDialogueText == null)
		{
			return false;
		}

		var utf8 = Encoding.UTF8.GetBytes(text.ToString());
		fixed (byte* pText = utf8)
		{
			ui.SetDialogueText(elementHandle, pText);
		}

		return true;
	}

	/// <summary>以 managed 字串顯示對白。</summary>
	public static bool PresentLine(ulong elementHandle, string text)
	{
		ArgumentNullException.ThrowIfNull(text);
		return PresentLine(elementHandle, text.AsSpan());
	}
}
