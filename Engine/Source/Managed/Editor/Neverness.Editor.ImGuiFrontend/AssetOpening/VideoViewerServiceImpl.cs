using Neverness.Editor.Assets.AssetOpening;
using Neverness.Editor.Core.Public;
using Neverness.Editor.ImGuiEx;
using Neverness.Editor.Media;

namespace Neverness.Editor.ImGuiFrontend.AssetOpening;

/// <summary>
/// 视频查看器服务实现——创建 ImVideoViewerWindow。
/// </summary>
public class VideoViewerServiceImpl : IVideoViewerService
{
    private readonly IImWindowManager _windowManager;
    private readonly AssetEditorManager _editorManager;

    public VideoViewerServiceImpl(IImWindowManager windowManager, AssetEditorManager editorManager)
    {
        _windowManager = windowManager;
        _editorManager = editorManager;
    }

    public void OpenViewer(VideoViewerOptions options)
    {
        var viewer = _windowManager.OpenWindow<ImVideoViewerWindow>(v =>
        {
            v.AssetGuidHex = options.AssetGuidHex;
            v.AssetName = options.AssetName;
            v.MetaInfo = options.MetaInfo;

            if (options.ThumbnailHandle != 0)
                v.SetThumbnail(options.ThumbnailHandle, options.ThumbnailSize);
        });

        if (options.AssetGuidHex != null)
        {
            var guid = Runtime.Assets.GUID.Parse(options.AssetGuidHex);
            _editorManager.Register(guid, viewer.WindowId);
        }
    }
}
