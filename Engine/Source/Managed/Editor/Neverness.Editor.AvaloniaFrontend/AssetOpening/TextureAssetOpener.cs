using Avalonia.Threading;
using Neverness.Editor.Assets;
using Neverness.Editor.Assets.AssetOpening;
using Neverness.Editor.AvaloniaFrontend.Dock;
using Neverness.Editor.AvaloniaFrontend.Public;
using Neverness.Editor.AvaloniaFrontend.Views;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;
using Neverness.Runtime.VFS.Public;

namespace Neverness.Editor.AvaloniaFrontend.AssetOpening;

/// <summary>
/// 纹理资产打开器——在浮动窗口中查看纹理资产。
///
/// 流程：
///   1. 检查 AssetEditorManager 是否已有该资产的窗口 → 聚焦
///   2. 后台线程：同步加载已导入的资产，若未导入则先导入源文件再加载
///   3. 主线程：创建独立浮动窗口（可拖拽停靠到主窗口）
/// </summary>
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
        // 1. 检查是否已有窗口（暂时跳过聚焦）
        if (_editorManager.TryGetWindowId(context.Guid, out var existingWindowId))
        {
            Console.WriteLine($"[TextureAssetOpener] 窗口已存在: {context.VirtualPath}");
            // TODO: 聚焦现有窗口
            return;
        }

        // 2. 后台加载资产（IO + 导入，避免阻塞 UI 线程）
        var (assetHandle, effectiveGuid) = await Task.Run(() => LoadOrImportAsset(context));

        if (assetHandle.IsZero)
        {
            Console.WriteLine($"[TextureAssetOpener] 加载资产失败: {context.VirtualPath} (GUID={context.Guid.ToHexString()}, TypeId={context.AssetTypeId})");
            return;
        }

        // 3. 在 UI 线程创建浮动窗口
        var assetName = context.VirtualPath.FileNameWithoutExtension;
        Console.WriteLine($"[TextureAssetOpener] 打开纹理查看器: {assetName} (GUID={effectiveGuid.ToHexString()})");

        try
        {
            // 确保在 UI 线程执行
            if (!Dispatcher.UIThread.CheckAccess())
            {
                await Dispatcher.UIThread.InvokeAsync(() => CreateFloatingWindow(assetName, effectiveGuid, assetHandle));
            }
            else
            {
                CreateFloatingWindow(assetName, effectiveGuid, assetHandle);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[TextureAssetOpener] 打开窗口异常: {ex}");
            assetHandle.Release();
        }
    }

    /// <summary>在 UI 线程创建浮动窗口。</summary>
    private void CreateFloatingWindow(string assetName, GUID effectiveGuid, AssetHandle assetHandle)
    {
        try
        {
            // 创建纹理查看器控件
            var viewerControl = new TextureViewerControl(assetName, effectiveGuid, _editorManager);

            // 加载纹理数据
            viewerControl.LoadFromAssetHandle(assetHandle);

            // 创建独立浮动窗口
            var window = new TextureViewerFloatingWindow(assetName, effectiveGuid, viewerControl);

            // 显示窗口
            window.Show();

            // 注册 Asset→Window 映射（使用 GUID 的 HashCode 作为窗口标识）
            var windowId = new Guid((int)(effectiveGuid.High >> 32), (short)(effectiveGuid.High >> 16), (short)effectiveGuid.High,
                (byte)(effectiveGuid.Low >> 56), (byte)(effectiveGuid.Low >> 48), (byte)(effectiveGuid.Low >> 40), (byte)(effectiveGuid.Low >> 32),
                (byte)(effectiveGuid.Low >> 24), (byte)(effectiveGuid.Low >> 16), (byte)(effectiveGuid.Low >> 8), (byte)effectiveGuid.Low);
            _editorManager.Register(effectiveGuid, windowId);

            // 释放 AssetHandle（控件已加载数据）
            assetHandle.Release();
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[TextureAssetOpener] 创建窗口失败: {ex}");
            assetHandle.Release();
        }
    }

    /// <summary>
    /// 尝试加载已导入的资产。若失败，先导入源文件再重试。
    /// 返回 (AssetHandle, 有效GUID)。导入后 GUID 可能与原始不同。
    /// </summary>
    private static (AssetHandle Handle, GUID EffectiveGuid) LoadOrImportAsset(AssetOpenContext context)
    {
        // 先尝试直接加载已导入的 .nnasset
        var handle = AssetHandle.LoadSync(context.Guid, context.AssetTypeId);
        if (!handle.IsZero)
            return (handle, context.Guid);

        // 加载失败 → 尝试导入源文件
        var absolutePath = VFS.GetAbsolutePath(context.VirtualPath.FullPath);
        if (absolutePath == null)
        {
            Console.WriteLine($"[TextureAssetOpener] 无法解析绝对路径: {context.VirtualPath}");
            return (AssetHandle.Zero, context.Guid);
        }

        var sourcePath = new NPath(absolutePath);
        if (!File.Exists(sourcePath.FullPath))
        {
            Console.WriteLine($"[TextureAssetOpener] 源文件不存在: {sourcePath}");
            return (AssetHandle.Zero, context.Guid);
        }

        Console.WriteLine($"[TextureAssetOpener] 资产未导入，正在导入: {sourcePath}");
        var importResult = Neverness.Editor.Assets.ImportPipeline.Import(sourcePath);

        if (!importResult.Success)
        {
            Console.WriteLine($"[TextureAssetOpener] 导入失败: {sourcePath} → {importResult.ErrorMessage}");
            return (AssetHandle.Zero, context.Guid);
        }

        // 导入成功 → 使用导入结果中的 GUID 重新加载
        var effectiveGuid = importResult.AssetGuid.IsZero ? context.Guid : importResult.AssetGuid;
        Console.WriteLine($"[TextureAssetOpener] 导入成功: {context.VirtualPath} (GUID={effectiveGuid.ToHexString()})");

        handle = AssetHandle.LoadSync(effectiveGuid, context.AssetTypeId);
        if (handle.IsZero)
        {
            Console.WriteLine($"[TextureAssetOpener] 导入后加载仍失败: {context.VirtualPath} (GUID={effectiveGuid.ToHexString()})");
        }

        return (handle, effectiveGuid);
    }
}
