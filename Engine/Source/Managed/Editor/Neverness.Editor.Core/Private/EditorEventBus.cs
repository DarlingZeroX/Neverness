namespace Neverness.Editor.Core.Public;

/// <summary>
/// 编辑器事件总线实现——同步 + 延迟分发。
/// 递归 Emit 自动降级为 deferred，防止栈溢出。
/// </summary>
public sealed class EditorEventBus : IEditorEventBus
{
    private readonly Dictionary<EditorEventType, List<Action<EditorEvent>>> _handlers = new();
    private readonly List<Action<EditorEvent>> _globalHandlers = new();
    private readonly Queue<EditorEvent> _deferredQueue = new();
    private bool _emitting;

    public void Subscribe(EditorEventType type, Action<EditorEvent> handler)
    {
        if (!_handlers.TryGetValue(type, out var list))
        {
            list = new List<Action<EditorEvent>>();
            _handlers[type] = list;
        }
        list.Add(handler);
    }

    public void Unsubscribe(EditorEventType type, Action<EditorEvent> handler)
    {
        if (_handlers.TryGetValue(type, out var list))
        {
            list.Remove(handler);
        }
    }

    public void SubscribeAll(Action<EditorEvent> handler) => _globalHandlers.Add(handler);

    public void UnsubscribeAll(Action<EditorEvent> handler) => _globalHandlers.Remove(handler);

    public void Emit(EditorEvent evt)
    {
        // 递归 Emit 自动降级为 deferred
        if (_emitting)
        {
            EmitDeferred(evt);
            return;
        }

        _emitting = true;
        try
        {
            Dispatch(evt);
        }
        finally
        {
            _emitting = false;
        }
    }

    public void EmitDeferred(EditorEvent evt) => _deferredQueue.Enqueue(evt);

    public void FlushDeferred()
    {
        while (_deferredQueue.Count > 0)
        {
            var evt = _deferredQueue.Dequeue();
            Emit(evt);
        }
    }

    public void Clear()
    {
        _handlers.Clear();
        _globalHandlers.Clear();
        _deferredQueue.Clear();
    }

    private void Dispatch(EditorEvent evt)
    {
        // 全局处理器
        foreach (var handler in _globalHandlers)
        {
            handler(evt);
        }

        // 类型特定处理器
        if (_handlers.TryGetValue(evt.Type, out var list))
        {
            foreach (var handler in list)
            {
                handler(evt);
            }
        }
    }
}
