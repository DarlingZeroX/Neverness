using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 资产编辑器窗口映射管理器。
/// 维护 Asset GUID → Window ID 的双向映射，实现"重复打开聚焦已有窗口"。
///
/// 由 <see cref="TextureAssetOpener"/> 等 Opener 在打开窗口后注册映射。
/// 窗口关闭时通过 <see cref="Unregister"/> 清除映射。
/// </summary>
public sealed class AssetEditorManager
{
    private readonly Dictionary<GUID, Guid> _assetToWindow = new();
    private readonly Dictionary<Guid, GUID> _windowToAsset = new();

    /// <summary>注册资产→窗口映射。</summary>
    public void Register(GUID assetGuid, Guid windowId)
    {
        _assetToWindow[assetGuid] = windowId;
        _windowToAsset[windowId] = assetGuid;
    }

    /// <summary>查找资产对应的已打开窗口 ID。</summary>
    public bool TryGetWindowId(GUID assetGuid, out Guid windowId)
    {
        return _assetToWindow.TryGetValue(assetGuid, out windowId);
    }

    /// <summary>查找窗口对应的资产 GUID。</summary>
    public bool TryGetAssetGuid(Guid windowId, out GUID assetGuid)
    {
        return _windowToAsset.TryGetValue(windowId, out assetGuid);
    }

    /// <summary>移除窗口映射（窗口关闭时调用）。</summary>
    public void Unregister(Guid windowId)
    {
        if (_windowToAsset.Remove(windowId, out var assetGuid))
        {
            _assetToWindow.Remove(assetGuid);
        }
    }

    /// <summary>移除资产映射。</summary>
    public void UnregisterAsset(GUID assetGuid)
    {
        if (_assetToWindow.Remove(assetGuid, out var windowId))
        {
            _windowToAsset.Remove(windowId);
        }
    }

    /// <summary>清除所有映射。</summary>
    public void Clear()
    {
        _assetToWindow.Clear();
        _windowToAsset.Clear();
    }
}
