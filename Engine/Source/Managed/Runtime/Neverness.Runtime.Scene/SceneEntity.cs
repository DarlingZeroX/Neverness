using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Scene;

/// <summary>
/// 场景实体薄门面：持有 Native <see cref="NNEntityHandle"/> 和所属场景句柄，
/// 属性经 <see cref="SceneNativeBridge"/> 泛型方法读写 ECS 组件。
/// </summary>
public sealed class SceneEntity
{
    /// <summary>Native ECS 实体句柄。</summary>
    public NNEntityHandle Handle { get; private set; }

    /// <summary>所属 Native 场景句柄（ulong，对齐 uint64）。</summary>
    public ulong SceneHandle { get; private set; }

    /// <summary>显示名称（仅作本地缓存，Native 端通过 Tag 组件存储）。</summary>
    public string DisplayName { get; set; }

    /// <summary>句柄非零时视为可参与场景 API 操作。</summary>
    public bool IsAlive => Handle.Value != 0;

    /// <summary>由已有 Native 句柄建立门面。</summary>
    public SceneEntity(NNEntityHandle handle, ulong sceneHandle = 0, string displayName = "Entity")
    {
        Handle = handle;
        SceneHandle = sceneHandle;
        DisplayName = displayName ?? string.Empty;
    }

    // ── 组件操作（泛型封装）──

    /// <summary>添加组件。</summary>
    public NNSceneResult AddComponent<T>() where T : struct =>
        SceneNativeBridge.AddComponent<T>(SceneHandle, Handle);

    /// <summary>移除组件。</summary>
    public NNSceneResult RemoveComponent<T>() where T : struct =>
        SceneNativeBridge.RemoveComponent<T>(SceneHandle, Handle);

    /// <summary>查询是否拥有组件。</summary>
    public bool HasComponent<T>() where T : struct =>
        SceneNativeBridge.HasComponent<T>(SceneHandle, Handle);

    /// <summary>读取组件数据；无组件时返回 null。</summary>
    public T? GetComponent<T>() where T : struct =>
        SceneNativeBridge.GetComponent<T>(SceneHandle, Handle);

    /// <summary>写入组件数据。</summary>
    public NNSceneResult SetComponent<T>(T data) where T : struct =>
        SceneNativeBridge.SetComponent<T>(SceneHandle, Handle, data);

    // ── 层级 ──

    /// <summary>设置父实体。</summary>
    public NNSceneResult SetParent(SceneEntity parent) =>
        SceneNativeBridge.SetParent(SceneHandle, Handle, parent.Handle);

    /// <summary>获取父实体句柄；无父时返回零句柄。</summary>
    public NNEntityHandle GetParent() =>
        SceneNativeBridge.GetParent(SceneHandle, Handle);

    // ── 内部 ──

    /// <summary>使句柄失效（由 <see cref="SceneManager.DestroyEntity"/> 调用）。</summary>
    internal void Invalidate()
    {
        Handle = default;
    }
}
