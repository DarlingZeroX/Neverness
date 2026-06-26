namespace Neverness.Editor.Core.Public;

/// <summary>
/// 编辑器事件类型枚举。
/// </summary>
public enum EditorEventType
{
    SelectionChanged,
    AssetOpened,
    AssetCreated,
    AssetDeleted,
    AssetRenamed,
    AssetReloaded,
    SceneOpened,
    SceneClosed,
    PlayModeChanged,
    ModuleInstalled,
    ShowToast,
}

/// <summary>
/// 编辑器事件数据。
/// </summary>
public readonly record struct EditorEvent(
    EditorEventType Type,
    object? Payload = null);

/// <summary>
/// 编辑器事件总线——支持同步和延迟分发。
/// Feature 通过此接口订阅和发送编辑器级事件。
/// </summary>
public interface IEditorEventBus
{
    void Subscribe(EditorEventType type, Action<EditorEvent> handler);
    void Unsubscribe(EditorEventType type, Action<EditorEvent> handler);
    void SubscribeAll(Action<EditorEvent> handler);
    void UnsubscribeAll(Action<EditorEvent> handler);
    void Emit(EditorEvent evt);
    void EmitDeferred(EditorEvent evt);
    void FlushDeferred();
    void Clear();
}
