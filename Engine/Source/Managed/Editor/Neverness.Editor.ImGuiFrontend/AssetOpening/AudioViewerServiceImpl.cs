using Neverness.Editor.Assets.AssetOpening;
using Neverness.Editor.Core.Public;
using Neverness.Editor.ImGuiEx;
using Neverness.Editor.Media;

namespace Neverness.Editor.ImGuiFrontend.AssetOpening;

/// <summary>
/// 音频查看器服务实现——创建 ImAudioViewerWindow。
/// </summary>
public class AudioViewerServiceImpl : IAudioViewerService
{
    private readonly IImWindowManager _windowManager;
    private readonly AssetEditorManager _editorManager;

    public AudioViewerServiceImpl(IImWindowManager windowManager, AssetEditorManager editorManager)
    {
        _windowManager = windowManager;
        _editorManager = editorManager;
    }

    public void OpenViewer(AudioViewerOptions options)
    {
        var viewer = _windowManager.OpenWindow<ImAudioViewerWindow>(v =>
        {
            v.AssetGuidHex = options.AssetGuidHex;
            v.AssetName = options.AssetName;
            v.MetaInfo = options.MetaInfo;

            if (options.Player != null)
                v.SetPlayer(options.Player);

            if (options.WaveformHandle != 0)
                v.SetWaveform(options.WaveformHandle, options.WaveformSize);
        });

        if (options.AssetGuidHex != null)
        {
            var guid = Runtime.Assets.GUID.Parse(options.AssetGuidHex);
            _editorManager.Register(guid, viewer.WindowId);
        }
    }
}
