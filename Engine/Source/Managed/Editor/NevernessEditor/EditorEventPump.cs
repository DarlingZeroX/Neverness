using Neverness.Editor.Core.Public;
using Neverness.Runtime.Application.Public;

namespace NevernessEditor;

/// <summary>
/// 编辑器事件泵——桥接 SDL 窗口事件到 Editor 系统。
/// 直接订阅 SdlWindowEvents，由 SdlEventBridge 自动路由。
/// </summary>
/// <remarks>
/// 集成点：EditorApplicationRunner 主循环中，ApplicationHost.PumpEvents() 之后、BeginFrame() 之前调用 <see cref="PollAndDispatch"/>。
/// </remarks>
public sealed class EditorEventPump : IDisposable
{
	private readonly SdlWindowEvents _windowEvents;
	private readonly IEditorEventBus _editorEvents;

	/// <summary>是否请求安全退出（收到 WindowClose 或 Quit 时设置）。</summary>
	public bool QuitRequested { get; private set; }

	/// <summary>编辑器是否拥有焦点。</summary>
	public bool HasFocus { get; private set; } = true;

	public EditorEventPump(SdlWindowEvents windowEvents, IEditorEventBus editorEvents)
	{
		ArgumentNullException.ThrowIfNull(windowEvents);
		ArgumentNullException.ThrowIfNull(editorEvents);

		_windowEvents = windowEvents;
		_editorEvents = editorEvents;

		// 订阅窗口事件
		_windowEvents.OnCloseRequested += HandleWindowClose;
		_windowEvents.OnResized += HandleWindowResize;
		_windowEvents.OnFocusChanged += HandleFocusChanged;
		_windowEvents.OnDropFile += HandleDropFile;
	}

	/// <summary>消费本帧所有事件并分发到 Editor 系统。</summary>
	public void PollAndDispatch()
	{
		// 事件由 SdlEventBridge 自动路由到 SdlWindow.Events，
	// 再通过订阅回调分发到此处。无需主动轮询。
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
		_windowEvents.OnCloseRequested -= HandleWindowClose;
		_windowEvents.OnResized -= HandleWindowResize;
		_windowEvents.OnFocusChanged -= HandleFocusChanged;
		_windowEvents.OnDropFile -= HandleDropFile;
	}

	/* ── 事件处理 ── */

	private void HandleWindowClose()
	{
		QuitRequested = true;
	}

	private void HandleWindowResize(int width, int height)
	{
		// TODO: 同步 ViewportPanel 尺寸
	}

	private void HandleFocusChanged(bool gained)
	{
		HasFocus = gained;
	}

	private void HandleDropFile(string filePath)
	{
		_editorEvents.Emit(new EditorEvent(
			EditorEventType.AssetCreated,
			Payload: filePath));
	}
}
