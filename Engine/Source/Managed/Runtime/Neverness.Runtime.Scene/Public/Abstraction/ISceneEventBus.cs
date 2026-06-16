namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景事件总线接口——与 ECS 实现无关。
/// 支持即时分发和延迟队列两种模式。
/// </summary>
public interface ISceneEventBus
{
    // ── 即时分发 ──

    /// <summary>即时分发泛型事件。</summary>
    void Publish<T>(T evt) where T : struct;

    // ── 延迟分发 ──

    /// <summary>延迟分发泛型事件。</summary>
    void PublishDeferred<T>(T evt) where T : struct;

    /// <summary>处理所有延迟事件。</summary>
    void FlushDeferred();

    // ── 订阅管理 ──

    /// <summary>订阅泛型事件。</summary>
    void Subscribe<T>(Action<T> handler) where T : struct;

    /// <summary>取消订阅泛型事件。</summary>
    void Unsubscribe<T>(Action<T> handler) where T : struct;

    /// <summary>获取泛型事件订阅者数量。</summary>
    int GetSubscriberCount<T>() where T : struct;

    /// <summary>清空所有订阅和延迟队列。</summary>
    void Clear();
}
