using Avalonia.Controls;
using Dock.Model.Mvvm.Controls;
using Neverness.Editor.AvaloniaFrontend.Public;
using Neverness.Editor.AvaloniaFrontend.Views;
using Neverness.Editor.Assets.AssetOpening;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.AvaloniaFrontend.Dock;

/// <summary>
/// Shared entry point for dockable asset editors hosted as dynamic Dock documents.
/// </summary>
public sealed class DockableAssetEditorFramework
{
    private readonly MainEditorWindow _mainWindow;
    private readonly EditorDockFactory _dockFactory;
    private readonly AssetEditorManager _assetEditorManager;

    public DockableAssetEditorFramework(
        MainEditorWindow mainWindow,
        EditorDockFactory dockFactory,
        AssetEditorManager assetEditorManager)
    {
        _mainWindow = mainWindow;
        _dockFactory = dockFactory;
        _assetEditorManager = assetEditorManager;
    }

    public bool TryActivateExisting(GUID assetGuid)
    {
        if (!_assetEditorManager.TryGetEditorId(assetGuid, out var editorId))
        {
            return false;
        }

        var panelId = GetPanelId(editorId);
        var document = _dockFactory.FindDocument(panelId);
        if (document == null)
        {
            _assetEditorManager.UnregisterEditor(editorId);
            _dockFactory.RemoveTextureViewerPanel(panelId);
            return false;
        }

        return _mainWindow.TryActivateDocument(panelId);
    }

    public Document OpenTextureEditor(string assetName, GUID assetGuid, Control content)
    {
        var editorId = ToEditorId(assetGuid);
        var panelId = GetPanelId(editorId);

        if (_dockFactory.TextureViewerDocuments.TryGetValue(panelId, out var existing))
        {
            existing.Context ??= content;
            _mainWindow.ShowDocument(existing, (Control)existing.Context!);
            _assetEditorManager.RegisterEditor(assetGuid, editorId);
            return existing;
        }

        var document = _dockFactory.CreateTextureViewerDocument(assetName, editorId);
        _mainWindow.ShowDocument(document, content);
        _assetEditorManager.RegisterEditor(assetGuid, editorId);
        return document;
    }

    public void CloseTextureEditor(Guid editorId)
    {
        var panelId = GetPanelId(editorId);
        _mainWindow.RemoveDocument(panelId);
        _dockFactory.RemoveTextureViewerPanel(panelId);
        _assetEditorManager.UnregisterEditor(editorId);
    }

    public static Guid ToEditorId(GUID assetGuid)
    {
        Span<byte> bytes = stackalloc byte[16];
        System.Buffers.Binary.BinaryPrimitives.WriteUInt64BigEndian(bytes[..8], assetGuid.High);
        System.Buffers.Binary.BinaryPrimitives.WriteUInt64BigEndian(bytes[8..], assetGuid.Low);
        return new Guid(bytes);
    }

    public static string GetPanelId(Guid editorId)
    {
        return $"{EditorDockFactory.PanelIds.TextureViewerPrefix}{editorId:D}";
    }
}
