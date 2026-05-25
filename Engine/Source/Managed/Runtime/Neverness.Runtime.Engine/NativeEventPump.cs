using System;
using System.Runtime.InteropServices;

namespace Neverness.Runtime.Engine;

/// <summary>
/// Native 事件泵——每帧调用 <see cref="PollEvents"/> 批量消费 NNEvent 队列。
/// 封装 <c>NNEventAPI</c> 函数表，提供类型化的事件分发。
/// </summary>
/// <remarks>
/// 架构：
/// - SDL 主线程（Native）将 SDL_Event 翻译为 NNEvent 并推入 SPSC 队列
/// - C# 主线程每帧调用 PollEvents() 批量出队
/// - 字符串（DROP_FILE 路径等）通过 Native String Pool 传递，下次 poll 前有效
/// - 两级分发：type（粗分类）→ subtype（细分类）
/// </remarks>
public sealed unsafe class NativeEventPump : IDisposable
{
	private readonly delegate* unmanaged<NNEvent*, uint> _pollEvent;
	private readonly delegate* unmanaged<NNEvent*, uint> _peekEvent;
	private readonly delegate* unmanaged<NNEvent*, byte**, ushort*, uint> _getEventString;
	private readonly delegate* unmanaged<uint> _getQueueCount;
	private readonly delegate* unmanaged<void> _flushEvents;
	private readonly delegate* unmanaged<NNEvent*, uint> _pushUserEvent;

	private bool _disposed;

	/// <summary>事件到达时触发。回调应在同一帧内完成，下次 Poll 前 String Pool 可能被覆盖。</summary>
	public event Action<NNEvent>? OnEvent;

	/// <summary>DROP_FILE 事件到达时触发，附带 UTF-8 路径字符串。</summary>
	public event Action<NNEvent, string>? OnDropFile;

	/// <summary>DROP_TEXT 事件到达时触发。</summary>
	public event Action<NNEvent, string>? OnDropText;

	/// <summary>TEXT_INPUT 事件到达时触发。</summary>
	public event Action<NNEvent, string>? OnTextInput;

	/// <summary>
	/// 从已安装的 <see cref="EngineNativeApiCache"/> 创建事件泵。
	/// </summary>
	public NativeEventPump()
	{
		ref readonly var api = ref EngineNativeApiCache.EngineApi;
		_pollEvent = api.Events.PollEvent;
		_peekEvent = api.Events.PeekEvent;
		_getEventString = api.Events.GetEventString;
		_getQueueCount = api.Events.GetQueueCount;
		_flushEvents = api.Events.FlushEvents;
		_pushUserEvent = api.Events.PushUserEvent;
	}

	/// <summary>
	/// 批量消费当前队列中的所有事件。
	/// 通常在每帧主循环开始时调用。
	/// </summary>
	public void PollEvents()
	{
		if (_disposed) return;

		NNEvent ev = default;
		while (_pollEvent(&ev) != 0)
		{
			DispatchEvent(ev);
		}
	}

	/// <summary>窥视下一个事件但不消费。</summary>
	public bool PeekEvent(out NNEvent ev)
	{
		ev = default;
		if (_disposed) return false;

		fixed (NNEvent* pEv = &ev)
		{
			return _peekEvent(pEv) != 0;
		}
	}

	/// <summary>获取队列中当前事件数量（近似值）。</summary>
	public uint GetQueueCount()
	{
		if (_disposed) return 0;
		return _getQueueCount();
	}

	/// <summary>清空事件队列。</summary>
	public void FlushEvents()
	{
		if (_disposed) return;
		_flushEvents();
	}

	/// <summary>推送用户自定义事件（C# → Native）。</summary>
	public bool PushUserEvent(NNEvent ev)
	{
		if (_disposed) return false;
		return _pushUserEvent(&ev) != 0;
	}

	/// <summary>
	/// 从事件的 String Pool 中读取 UTF-8 字符串。
	/// 字符串在下次 <see cref="PollEvents"/> 调用前有效。
	/// </summary>
	public string? ReadEventString(in NNEvent ev)
	{
		if (_disposed) return null;

		byte* ptr = null;
		ushort len = 0;
		fixed (NNEvent* pEv = &ev)
		{
			if (_getEventString(pEv, &ptr, &len) == 0 || ptr == null || len == 0)
				return null;
		}

		return System.Text.Encoding.UTF8.GetString(ptr, len);
	}

	private void DispatchEvent(in NNEvent ev)
	{
		OnEvent?.Invoke(ev);

		switch ((NNEventType)ev.Type)
		{
			case NNEventType.Window:
				/* 窗口事件无字符串变体，由 EditorEventPump 处理 */
				break;

			case NNEventType.Input:
				DispatchInputEvent(ev);
				break;

			case NNEventType.System:
				/* 系统事件由 EditorEventPump 处理 */
				break;
		}
	}

	private void DispatchInputEvent(in NNEvent ev)
	{
		switch ((NNInputEventSubtype)ev.Subtype)
		{
			case NNInputEventSubtype.DropFile:
			{
				var str = ReadEventString(ev);
				if (str != null)
					OnDropFile?.Invoke(ev, str);
				break;
			}
			case NNInputEventSubtype.DropText:
			{
				var str = ReadEventString(ev);
				if (str != null)
					OnDropText?.Invoke(ev, str);
				break;
			}
			case NNInputEventSubtype.TextInput:
			{
				var str = ReadEventString(ev);
				if (str != null)
					OnTextInput?.Invoke(ev, str);
				break;
			}
		}
	}

	public void Dispose()
	{
		if (_disposed) return;
		_disposed = true;
		FlushEvents();
	}
}
