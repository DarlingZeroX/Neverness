using System.Runtime.InteropServices;
using Neverness.Runtime.Scene.Internal;

namespace Neverness.Runtime.Scene;

/// <summary>
/// Native 事件桥接——将 Native <c>NNSceneEventBus</c> 的事件转发到 C# <see cref="SceneEventBus"/>。
///
/// 当前为 Phase 4-I 基础实现：提供回调注册基础设施和手动事件注入。
/// 待 NNSceneApi ABI 新增事件订阅函数指针（layoutVersion 7+）后，启用自动桥接。
/// </summary>
public sealed class NativeEventBridge : IDisposable
{
    private readonly SceneEventBus _eventBus;
    private readonly ulong _sceneHandle;
    private bool _disposed;

    /// <summary>Native 回调委托（保持引用防止 GC 回收）。</summary>
    private NativeEventCallback? _callbackDelegate;

    /// <summary>
    /// Native 事件回调签名——与未来 NNSceneApi 事件订阅 ABI 对齐。
    /// </summary>
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void NativeEventCallback(
        byte eventType, ulong entity, ulong otherEntity, ulong componentTypeId);

    public NativeEventBridge(ulong sceneHandle, SceneEventBus eventBus)
    {
        _sceneHandle = sceneHandle;
        _eventBus = eventBus;
    }

    /// <summary>底层场景句柄。</summary>
    public ulong SceneHandle => _sceneHandle;

    /// <summary>
    /// 启用 Native 事件自动桥接。
    /// 待 NNSceneApi 新增 <c>SubscribeEvents</c> 函数指针后生效。
    /// 当前可手动调用 <see cref="InjectEvent"/> 注入事件。
    /// </summary>
    public void EnableAutoBridge()
    {
        if (_disposed)
        {
            return;
        }

        _callbackDelegate = OnNativeEvent;

        // TODO: 待 NNSceneApi ABI 扩展后取消注释
        // 固定委托指针防止 GC 回收
        // var fnPtr = Marshal.GetFunctionPointerForDelegate(_callbackDelegate);
        // SceneNativeBridge.SubscribeEvents(_sceneHandle, fnPtr);
    }

    /// <summary>
    /// 手动注入事件到 C# 事件总线。
    /// 用于测试或从非 Native 来源触发事件（如 Editor 操作）。
    /// </summary>
    public void InjectEvent(SceneEvent evt)
    {
        if (_disposed)
        {
            return;
        }

        _eventBus.Emit(evt);
    }

    /// <summary>释放桥接资源。</summary>
    public void Dispose()
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;

        // TODO: 待 ABI 扩展后取消订阅
        // SceneNativeBridge.UnsubscribeEvents(_sceneHandle);
        _callbackDelegate = null;
    }

    /// <summary>Native 回调入口——将 Native 事件参数转换为 C# SceneEvent 并分发。</summary>
    private void OnNativeEvent(byte eventType, ulong entity, ulong otherEntity, ulong componentTypeId)
    {
        if (_disposed)
        {
            return;
        }

        var type = (SceneEventType)eventType;
        var evt = new SceneEvent(
            type,
            new Engine.NNEntityHandle(entity),
            new Engine.NNEntityHandle(otherEntity),
            componentTypeId);

        _eventBus.Emit(evt);
    }
}
