using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// Tracks the relationship between an asset GUID and its active editor instance.
/// </summary>
public sealed class AssetEditorManager
{
    private readonly Dictionary<GUID, Guid> _assetToEditor = new();
    private readonly Dictionary<Guid, GUID> _editorToAsset = new();

    public void Register(GUID assetGuid, Guid windowId)
    {
        RegisterEditor(assetGuid, windowId);
    }

    public void RegisterEditor(GUID assetGuid, Guid editorId)
    {
        _assetToEditor[assetGuid] = editorId;
        _editorToAsset[editorId] = assetGuid;
    }

    public bool TryGetWindowId(GUID assetGuid, out Guid windowId)
    {
        return TryGetEditorId(assetGuid, out windowId);
    }

    public bool TryGetEditorId(GUID assetGuid, out Guid editorId)
    {
        return _assetToEditor.TryGetValue(assetGuid, out editorId);
    }

    public bool TryGetAssetGuid(Guid windowId, out GUID assetGuid)
    {
        return TryGetAssetGuidFromEditor(windowId, out assetGuid);
    }

    public bool TryGetAssetGuidFromEditor(Guid editorId, out GUID assetGuid)
    {
        return _editorToAsset.TryGetValue(editorId, out assetGuid);
    }

    public void Unregister(Guid windowId)
    {
        UnregisterEditor(windowId);
    }

    public void UnregisterEditor(Guid editorId)
    {
        if (_editorToAsset.Remove(editorId, out var assetGuid))
        {
            _assetToEditor.Remove(assetGuid);
        }
    }

    public void UnregisterAsset(GUID assetGuid)
    {
        if (_assetToEditor.Remove(assetGuid, out var editorId))
        {
            _editorToAsset.Remove(editorId);
        }
    }

    public void Clear()
    {
        _assetToEditor.Clear();
        _editorToAsset.Clear();
    }
}
