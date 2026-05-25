using Neverness.Editor.Core.Public;
using Neverness.Runtime.Engine;

namespace NevernessEditor;

/// <summary>
/// 编辑器事件泵——桥接 Native SDL 事件到 Editor 系统。
/// 使用两级 type + subtype 匹配。
/// </summary>
/// <remarks>
/// 集成点：EditorApplicationRunner 主循环中，ApplicationHost.PumpEvents() 之后、BeginFrame() 之前调用 <see cref="PollAndDispatch"/>。
/// </remarks>
public sealed class EditorEventPump : IDisposable
{
	private readonly NativeEventPump _nativePump;
	private readonly IEditorEventBus _editorEvents;

	/// <summary>是否请求安全退出（收到 WindowClose 或 Quit 时设置）。</summary>
	public bool QuitRequested { get; private set; }

	/// <summary>编辑器是否拥有焦点。</summary>
	public bool HasFocus { get; private set; } = true;

	public EditorEventPump(NativeEventPump nativePump, IEditorEventBus editorEvents)
	{
		ArgumentNullException.ThrowIfNull(nativePump);
		ArgumentNullException.ThrowIfNull(editorEvents);

		_nativePump = nativePump;
		_editorEvents = editorEvents;

		_nativePump.OnDropFile += HandleDropFile;
		_nativePump.OnTextInput += HandleTextInput;
	}

	/// <summary>消费本帧所有 Native 事件并分发到 Editor 系统。</summary>
	public void PollAndDispatch()
	{
		_nativePump.OnEvent += DispatchNativeEvent;
		_nativePump.PollEvents();
		_nativePump.OnEvent -= DispatchNativeEvent;
	}

	/// <summary>重置退出请求状态。</summary>
	public void CancelQuitRequest()
	{
		QuitRequested = false;
	}

	/// <summary>刷新延迟事件队列（每帧结束时调用）。</summary>
	public void FlushDeferred()
	{
		_editorEvents.FlushDeferred();
	}

	public void Dispose()
	{
		_nativePump.OnDropFile -= HandleDropFile;
		_nativePump.OnTextInput -= HandleTextInput;
		_nativePump.Dispose();
	}

	/* ── 两级事件分发 ── */

	private void DispatchNativeEvent(NNEvent ev)
	{
		var type = (NNEventType)ev.Type;

		if (type == NNEventType.Window)
		{
			var sub = (NNWindowEventSubtype)ev.Subtype;
			switch (sub)
			{
				case NNWindowEventSubtype.Close:
				case NNWindowEventSubtype.Terminating:
					HandleWindowClose(ev);
					break;

				case NNWindowEventSubtype.Resized:
				case NNWindowEventSubtype.PixelSizeChanged:
					HandleWindowResize(ev);
					break;

				case NNWindowEventSubtype.FocusGained:
					HandleFocusGained(ev);
					break;

				case NNWindowEventSubtype.FocusLost:
					HandleFocusLost(ev);
					break;

				case NNWindowEventSubtype.DpiChanged:
					HandleDpiChanged(ev);
					break;

				case NNWindowEventSubtype.Minimized:
					HandleMinimized(ev);
					break;

				case NNWindowEventSubtype.Restored:
					HandleRestored(ev);
					break;

				case NNWindowEventSubtype.LowMemory:
					HandleLowMemory(ev);
					break;
			}
		}
		else if (type == NNEventType.Input)
		{
			var sub = (NNInputEventSubtype)ev.Subtype;
			switch (sub)
			{
				case NNInputEventSubtype.DropBegin:
					HandleDropBegin(ev);
					break;

				case NNInputEventSubtype.DropComplete:
					HandleDropComplete(ev);
					break;

				/* DropFile/DropText/TextInput 由 NativeEventPump 字符串回调处理 */
			}
		}
		else if (type == NNEventType.System)
		{
			var sub = (NNSystemEventSubtype)ev.Subtype;
			if (sub == NNSystemEventSubtype.Quit)
				HandleWindowClose(ev);
		}
	}

	/* ── 具体事件处理 ── */

	private void HandleWindowClose(NNEvent ev)
	{
		QuitRequested = true;
	}

	private void HandleWindowResize(NNEvent ev)
	{
		int width = ev.Data1;
		int height = ev.Data2;
		// TODO: 同步 ViewportPanel 尺寸
	}

	private void HandleFocusGained(NNEvent ev)
	{
		HasFocus = true;
	}

	private void HandleFocusLost(NNEvent ev)
	{
		HasFocus = false;
	}

	private void HandleDpiChanged(NNEvent ev)
	{
		// TODO: 更新 ImGui 显示缩放
	}

	private void HandleMinimized(NNEvent ev)
	{
		// TODO: 暂停渲染循环
	}

	private void HandleRestored(NNEvent ev)
	{
		// TODO: 恢复渲染循环
	}

	private void HandleDropBegin(NNEvent ev)
	{
		// 拖放开始
	}

	private void HandleDropFile(NNEvent ev, string filePath)
	{
		// TODO: 完整的资产导入流程
		_editorEvents.Emit(new EditorEvent(
			EditorEventType.AssetCreated,
            Payload: filePath));
	}

	private void HandleDropComplete(NNEvent ev)
	{
		// 拖放结束
	}

	private void HandleTextInput(NNEvent ev, string text)
	{
		// TextInput 由 ImGui SDL3 后端直接处理
	}

	private void HandleLowMemory(NNEvent ev)
	{
		_editorEvents.EmitDeferred(new EditorEvent(
			EditorEventType.PlayModeChanged,
            Payload: "low_memory"));
	}
}
