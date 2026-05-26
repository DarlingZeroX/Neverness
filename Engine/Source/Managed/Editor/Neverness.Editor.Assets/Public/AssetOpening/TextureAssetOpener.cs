using System.Numerics;
using Neverness.Editor.ImGuiEx;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;
using Neverness.Runtime.VFS.Public;

namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 纹理资产打开器——打开 ImTextureViewerWindow 查看纹理资产。
///
/// 流程：
///   1. 检查 AssetEditorManager 是否已有该资产的窗口 → 聚焦
///   2. 后台线程：同步加载已导入的资产，若未导入则先导入源文件再加载
///   3. 主线程：GPU 上传 + ImGui Handle 获取 + 窗口创建（通过 UIThreadDispatcher 调度）
///
/// 关键：OpenGL 纹理创建必须在主线程执行（只有主线程有 GL 上下文）。
/// `await Task.Run` 之后的代码运行在主线程，但 GPU 操作需通过 UIThreadDispatcher
/// 调度到 ProcessQueue 中执行，确保与 GL 帧同步。
/// </summary>
[AssetOpener(AssetTypeId.Texture2D)]
public sealed class TextureAssetOpener : IAssetOpener
{
    private readonly IImWindowManager _windowManager;
    private readonly AssetEditorManager _editorManager;
    private readonly UIThreadDispatcher _dispatcher;

    public TextureAssetOpener(IImWindowManager windowManager, AssetEditorManager editorManager, UIThreadDispatcher dispatcher)
    {
        _windowManager = windowManager;
        _editorManager = editorManager;
        _dispatcher = dispatcher;
    }

    public bool CanOpen(ulong assetTypeId) => assetTypeId == AssetTypeId.Texture2D;

    public async Task OpenAsync(AssetOpenContext context)
    {
        // 1. 检查是否已有窗口
        if (_editorManager.TryGetWindowId(context.Guid, out var existingWindowId))
        {
            _windowManager.FocusWindow(existingWindowId);
            return;
        }

        // 2. 后台加载资产（IO + 导入，避免阻塞 UI 线程）
        var (assetHandle, effectiveGuid) = await Task.Run(() => LoadOrImportAsset(context));

        if (assetHandle.IsZero)
        {
            Console.WriteLine($"[TextureAssetOpener] 加载资产失败: {context.VirtualPath} (GUID={context.Guid.ToHexString()}, TypeId={context.AssetTypeId})");
            return;
        }

        // 3. GPU 操作必须在主线程执行（GL 上下文）
        //    通过 UIThreadDispatcher 调度到 ProcessQueue，用 TaskCompletionSource 等待完成
        var tcs = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
        var path = context.VirtualPath;
        _dispatcher.Enqueue(() =>
        {
            try
            {
                // 上传 GPU 纹理（C++ 内部：读 blob → 反序列化 → OpenGL Texture2D）
                var textureKey = TextureInterop.LoadTextureFromAsset(assetHandle.Value);
                if (textureKey == 0)
                {
                    Console.WriteLine($"[TextureAssetOpener] GPU 上传失败: {path}");
                    assetHandle.Release();
                    return;
                }

                // 获取 ImGui Handle + 纹理尺寸
                var imGuiHandle = TextureInterop.GetImGuiTextureHandle(textureKey);
                var (w, h) = TextureInterop.GetTextureSize(textureKey);

                if (imGuiHandle == 0)
                {
                    Console.WriteLine($"[TextureAssetOpener] ImGui Handle 无效: {path}");
                    TextureInterop.ReleaseTexture(textureKey);
                    assetHandle.Release();
                    return;
                }

                // 打开 TextureViewer 窗口
                var viewer = _windowManager.OpenWindow<ImTextureViewerWindow>(v =>
                {
                    v.AssetGuidHex = effectiveGuid.ToHexString();
                    v.AssetName = path.FileNameWithoutExtension;
                    v.SetTexture(imGuiHandle, new Vector2(w, h));
                });

                // 注册 Asset→Window 映射
                _editorManager.Register(effectiveGuid, viewer.WindowId);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[TextureAssetOpener] 主线程 GPU 操作异常: {ex}");
                assetHandle.Release();
            }
            finally
            {
                tcs.TrySetResult();
            }
        });

        // 等待主线程 ProcessQueue 执行完毕（下一帧处理）
        await tcs.Task;
    }

    /// <summary>
    /// 尝试加载已导入的资产。若失败，先导入源文件再重试。
    /// 返回 (AssetHandle, 有效GUID)。导入后 GUID 可能与原始不同。
    /// </summary>
    private static (AssetHandle Handle, GUID EffectiveGuid) LoadOrImportAsset(AssetOpenContext context)
    {
        // 先尝试直接加载已导入的 .nnasset
        var handle = AssetHandleExtensions.LoadSync(context.Guid, context.AssetTypeId);
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
        var importResult = ImportPipeline.Import(sourcePath);

        if (!importResult.Success)
        {
            Console.WriteLine($"[TextureAssetOpener] 导入失败: {sourcePath} → {importResult.ErrorMessage}");
            return (AssetHandle.Zero, context.Guid);
        }

        // 导入成功 → 使用导入结果中的 GUID 重新加载
        var effectiveGuid = importResult.AssetGuid.IsZero ? context.Guid : importResult.AssetGuid;
        Console.WriteLine($"[TextureAssetOpener] 导入成功: {context.VirtualPath} (GUID={effectiveGuid.ToHexString()})");

        handle = AssetHandleExtensions.LoadSync(effectiveGuid, context.AssetTypeId);
        if (handle.IsZero)
        {
            Console.WriteLine($"[TextureAssetOpener] 导入后加载仍失败: {context.VirtualPath} (GUID={effectiveGuid.ToHexString()})");
        }

        return (handle, effectiveGuid);
    }
}
