using System.Numerics;
using Neverness.Editor.Assets;
using Neverness.Editor.Assets.AssetOpening;
using Neverness.Editor.ImGuiEx;
using Neverness.Runtime.Assets;
using Neverness.Runtime.Engine;
using Neverness.Runtime.VFS.Public;

using ImportPipeline = Neverness.Editor.Assets.ImportPipeline;

namespace Neverness.Editor.Media;

/// <summary>
/// 视频资产打开器——双击视频资产打开视频预览窗口。
///
/// 流程：
///   1. 检查 AssetEditorManager 是否已有该资产的窗口 → 聚焦
///   2. 后台线程：加载资产 + 通过 MediaImportBridge 解码首帧缩略图
///   3. 主线程：GPU 上传 + ImGui 窗口创建
///
/// TODO: 完整视频播放需要 MediaRuntimeAPI 编译后实现。
/// </summary>
[AssetOpener(AssetTypeId.VideoClip)]
public sealed class VideoAssetOpener : IAssetOpener
{
    private readonly IImWindowManager _windowManager;
    private readonly AssetEditorManager _editorManager;
    private readonly UIThreadDispatcher _dispatcher;

    public VideoAssetOpener(
        IImWindowManager windowManager,
        AssetEditorManager editorManager,
        UIThreadDispatcher dispatcher)
    {
        _windowManager = windowManager;
        _editorManager = editorManager;
        _dispatcher = dispatcher;
    }

    public bool CanOpen(ulong assetTypeId) => assetTypeId == AssetTypeId.VideoClip;

    public async Task OpenAsync(AssetOpenContext context)
    {
        // 1. 检查是否已有窗口
        if (_editorManager.TryGetWindowId(context.Guid, out var existingWindowId))
        {
            _windowManager.FocusWindow(existingWindowId);
            return;
        }

        // 2. 后台加载资产 + 解码首帧
        var (handle, effectiveGuid, metaInfo, thumbnail) = await Task.Run(
            () => LoadVideoAsset(context));

        if (handle.IsZero)
        {
            Console.WriteLine($"[VideoAssetOpener] 加载失败: {context.VirtualPath}");
            return;
        }

        // 3. 主线程：创建窗口
        var tcs = new TaskCompletionSource(TaskCreationOptions.RunContinuationsAsynchronously);
        var path = context.VirtualPath;

        _dispatcher.Enqueue(() =>
        {
            try
            {
                var viewer = _windowManager.OpenWindow<ImVideoViewerWindow>(v =>
                {
                    v.AssetGuidHex = effectiveGuid.ToHexString();
                    v.AssetName = path.FileNameWithoutExtension;
                    v.MetaInfo = metaInfo;

                    if (thumbnail.Handle != 0)
                        v.SetThumbnail(thumbnail.Handle, thumbnail.Size);
                });

                _editorManager.Register(effectiveGuid, viewer.WindowId);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[VideoAssetOpener] 异常: {ex}");
            }
            finally
            {
                tcs.TrySetResult();
            }
        });

        await tcs.Task;
    }

    /// <summary>后台加载视频资产 + 解码首帧。</summary>
    private static (AssetHandle Handle, GUID Guid, VideoMetaInfo Meta, (ulong Handle, Vector2 Size) Thumb)
        LoadVideoAsset(AssetOpenContext context)
    {
        var handle = AssetHandleExtensions.LoadSync(context.Guid, AssetTypeId.VideoClip);
        if (handle.IsZero)
        {
            var importResult = TryImportSource(context);
            if (importResult.Handle.IsZero)
                return (AssetHandle.Zero, context.Guid, default, (0, Vector2.Zero));
            handle = importResult.Handle;
        }

        // 解析 TypeInfo（48 字节 NNVideoTypeInfo）
        var metaInfo = ParseVideoMetaInfo(handle);

        // 尝试通过 MediaImportBridge 解码首帧缩略图
        // C++ DLL 未就绪时优雅降级
        var thumbnail = (Handle: 0ul, Size: Vector2.Zero);
        try
        {
            var sourcePath = context.VirtualPath.FullPath;
            var decoded = MediaImportBridge.DecodeThumbnail(sourcePath, 256);
            if (decoded != null)
            {
                // 注意：GPU 纹理上传必须在主线程
                // 这里只保存像素数据，在主线程 _dispatcher.Enqueue 中上传
                // 暂时跳过——首帧纹理上传需要 TextureInterop 支持
                Console.WriteLine($"[VideoAssetOpener] 首帧解码: {decoded.Width}x{decoded.Height}");
            }
        }
        catch
        {
            // MediaImportBridge DLL 未就绪，忽略
        }

        return (handle, context.Guid, metaInfo, thumbnail);
    }

    /// <summary>从 AssetHandle 读取 TypeInfo 解析视频元信息。</summary>
    private static VideoMetaInfo ParseVideoMetaInfo(AssetHandle handle)
    {
        try
        {
            // TODO: 通过 blob type 查找 TypeInfo
            // 当前返回默认值
            return new VideoMetaInfo
            {
                Width = 0,
                Height = 0,
                FpsNum = 30000,
                FpsDen = 1001,
                FrameCount = 0,
                Duration = 0,
                CodecName = "Unknown",
                HasAudio = false
            };
        }
        catch
        {
            return default;
        }
    }

    /// <summary>尝试导入源文件。</summary>
    private static (AssetHandle Handle, GUID Guid) TryImportSource(AssetOpenContext context)
    {
        try
        {
            var absolutePath = VFS.GetAbsolutePath(context.VirtualPath.FullPath);
            if (absolutePath == null) return (AssetHandle.Zero, context.Guid);

            var sourcePath = new NPath(absolutePath);
            if (!File.Exists(sourcePath.FullPath)) return (AssetHandle.Zero, context.Guid);

            Console.WriteLine($"[VideoAssetOpener] 导入: {sourcePath}");
            var result = ImportPipeline.Import(sourcePath);
            if (!result.Success) return (AssetHandle.Zero, context.Guid);

            var effectiveGuid = result.AssetGuid.IsZero ? context.Guid : result.AssetGuid;
            var handle = AssetHandleExtensions.LoadSync(effectiveGuid, AssetTypeId.VideoClip);
            return (handle, effectiveGuid);
        }
        catch
        {
            return (AssetHandle.Zero, context.Guid);
        }
    }
}
