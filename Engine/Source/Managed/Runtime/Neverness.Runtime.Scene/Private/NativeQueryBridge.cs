using System.Runtime.InteropServices;
using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;

namespace Neverness.Runtime.Scene.Internal;

/// <summary>
/// Native Query ABI 桥接——封装批量查询调用，减少 P/Invoke 次数。
/// 调用方负责分配输出缓冲区，本类仅做 ABI 转发。
/// </summary>
internal static unsafe class NativeQueryBridge
{
    /// <summary>查询拥有指定组件的所有实体句柄。</summary>
    /// <returns>匹配数量（即使缓冲区不足也会返回总数）。</returns>
    public static uint QueryEntities(ulong sceneHandle, ulong componentTypeId, Span<ulong> outHandles)
    {
        if (sceneHandle == 0 || !TryGetSceneApi(out var api) || api.QueryEntities == null)
        {
            return 0;
        }

        uint count = 0;
        fixed (ulong* pHandles = outHandles)
        {
            api.QueryEntities(sceneHandle, componentTypeId, pHandles, (uint)outHandles.Length, &count);
        }
        return count;
    }

    /// <summary>获取拥有指定组件的实体数量（不获取数据）。</summary>
    public static uint QueryEntityCount(ulong sceneHandle, ulong componentTypeId)
    {
        if (sceneHandle == 0 || !TryGetSceneApi(out var api) || api.QueryEntities == null)
        {
            return 0;
        }

        uint count = 0;
        api.QueryEntities(sceneHandle, componentTypeId, null, 0, &count);
        return count;
    }

    /// <summary>批量读取组件数据到连续缓冲区。</summary>
    public static NNSceneResult QueryComponents(
        ulong sceneHandle,
        ulong componentTypeId,
        ReadOnlySpan<ulong> entities,
        Span<byte> outBuffer,
        uint componentSize)
    {
        if (sceneHandle == 0 || !TryGetSceneApi(out var api) || api.QueryComponents == null)
        {
            return NNSceneResult.Invalid;
        }

        fixed (ulong* pEntities = entities)
        fixed (byte* pData = outBuffer)
        {
            return api.QueryComponents(
                sceneHandle, componentTypeId,
                pEntities, (uint)entities.Length,
                pData, componentSize);
        }
    }

    /// <summary>查询同时拥有两个指定组件的实体数量。</summary>
    public static uint QueryCount2(ulong sceneHandle, ulong typeId1, ulong typeId2)
    {
        if (sceneHandle == 0 || !TryGetSceneApi(out var api) || api.QueryCount2 == null)
        {
            return 0;
        }

        uint count = 0;
        api.QueryCount2(sceneHandle, typeId1, typeId2, &count);
        return count;
    }

    private static bool TryGetSceneApi(out NNSceneApi api)
    {
        if (!EngineNativeApiBootstrap.IsInstalled)
        {
            api = default;
            return false;
        }

        api = EngineNativeApiBootstrap.EngineApi.Scene;
        return true;
    }
}
