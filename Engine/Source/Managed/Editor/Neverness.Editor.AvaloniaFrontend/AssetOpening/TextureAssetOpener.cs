using Avalonia.Threading;
using Neverness.Editor.Assets;
using Neverness.Editor.Assets.AssetOpening;
using Neverness.Editor.AvaloniaFrontend.Dock;
using Neverness.Editor.AvaloniaFrontend.Public;
using Neverness.Editor.AvaloniaFrontend.Views;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;
using Neverness.Runtime.VFS;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.AvaloniaFrontend.AssetOpening;

[AssetOpener(AssetTypeId.Texture2D)]
public sealed class TextureAssetOpener : IAssetOpener
{
    private readonly AssetEditorManager _editorManager;

    public TextureAssetOpener(AssetEditorManager editorManager)
    {
        _editorManager = editorManager;
    }

    public bool CanOpen(ulong assetTypeId) => assetTypeId == AssetTypeId.Texture2D;

    public async Task OpenAsync(AssetOpenContext context)
    {
        var editorFramework = AvaloniaFrontendModule.CreateAssetEditorFramework(_editorManager);
        if (editorFramework.TryActivateExisting(context.Guid))
        {
            Console.WriteLine($"[TextureAssetOpener] Existing texture editor activated: {context.VirtualPath}");
            return;
        }

        var (assetHandle, effectiveGuid) = await Task.Run(() => LoadOrImportAsset(context));
        if (assetHandle.IsZero)
        {
            Console.WriteLine($"[TextureAssetOpener] Failed to load asset: {context.VirtualPath} (GUID={context.Guid.ToHexString()}, TypeId={context.AssetTypeId})");
            return;
        }

        var assetName = context.VirtualPath.FileNameWithoutExtension;
        Console.WriteLine($"[TextureAssetOpener] Open texture viewer: {assetName} (GUID={effectiveGuid.ToHexString()})");

        try
        {
            if (!Dispatcher.UIThread.CheckAccess())
            {
                await Dispatcher.UIThread.InvokeAsync(() => OpenDockDocument(editorFramework, assetName, effectiveGuid, assetHandle));
            }
            else
            {
                OpenDockDocument(editorFramework, assetName, effectiveGuid, assetHandle);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[TextureAssetOpener] Open editor failed: {ex}");
            assetHandle.Release();
        }
    }

    private void OpenDockDocument(
        DockableAssetEditorFramework editorFramework,
        string assetName,
        GUID effectiveGuid,
        AssetHandle assetHandle)
    {
        try
        {
            var viewerControl = new TextureViewerControl(assetName, effectiveGuid, _editorManager);
            viewerControl.LoadFromAssetHandle(assetHandle);
            editorFramework.OpenTextureEditor(assetName, effectiveGuid, viewerControl);
            assetHandle.Release();
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[TextureAssetOpener] Create dock document failed: {ex}");
            assetHandle.Release();
        }
    }

    private static (AssetHandle Handle, GUID EffectiveGuid) LoadOrImportAsset(AssetOpenContext context)
    {
        var handle = AssetHandle.LoadSync(context.Guid, context.AssetTypeId);
        if (!handle.IsZero)
        {
            return (handle, context.Guid);
        }

        var absolutePath = VFSService.GetAbsolutePath(context.VirtualPath.FullPath);
        if (absolutePath == null)
        {
            Console.WriteLine($"[TextureAssetOpener] Failed to resolve absolute path: {context.VirtualPath}");
            return (AssetHandle.Zero, context.Guid);
        }

        var sourcePath = new NPath(absolutePath);
        if (!File.Exists(sourcePath.FullPath))
        {
            Console.WriteLine($"[TextureAssetOpener] Source file does not exist: {sourcePath}");
            return (AssetHandle.Zero, context.Guid);
        }

        Console.WriteLine($"[TextureAssetOpener] Asset not imported, importing now: {sourcePath}");
        var importResult = Neverness.Editor.Assets.ImportPipeline.Import(sourcePath);
        if (!importResult.Success)
        {
            Console.WriteLine($"[TextureAssetOpener] Import failed: {sourcePath} -> {importResult.ErrorMessage}");
            return (AssetHandle.Zero, context.Guid);
        }

        var effectiveGuid = importResult.AssetGuid.IsZero ? context.Guid : importResult.AssetGuid;
        Console.WriteLine($"[TextureAssetOpener] Import succeeded: {context.VirtualPath} (GUID={effectiveGuid.ToHexString()})");

        handle = AssetHandle.LoadSync(effectiveGuid, context.AssetTypeId);
        if (handle.IsZero)
        {
            Console.WriteLine($"[TextureAssetOpener] Load still failed after import: {context.VirtualPath} (GUID={effectiveGuid.ToHexString()})");
        }

        return (handle, effectiveGuid);
    }
}
