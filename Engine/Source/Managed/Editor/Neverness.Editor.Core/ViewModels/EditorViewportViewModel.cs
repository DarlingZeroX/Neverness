using Neverness.Editor.Framework.Public.Mvvm;

namespace Neverness.Editor.Core.ViewModels;

/// <summary>
/// 编辑器视口 ViewModel——暴露视口状态供 View 层渲染。
/// </summary>
public class EditorViewportViewModel : ViewModelBase
{
    private ulong _sceneHandle;
    private float _viewportWidth;
    private float _viewportHeight;
    private ulong _sceneTextureId;
    private ulong _rmluiTextureId;

    /// <summary>关联的场景句柄。</summary>
    public ulong SceneHandle
    {
        get => _sceneHandle;
        set => SetProperty(ref _sceneHandle, value);
    }

    /// <summary>视口宽度。</summary>
    public float ViewportWidth
    {
        get => _viewportWidth;
        set => SetProperty(ref _viewportWidth, value);
    }

    /// <summary>视口高度。</summary>
    public float ViewportHeight
    {
        get => _viewportHeight;
        set => SetProperty(ref _viewportHeight, value);
    }

    /// <summary>场景渲染纹理 ID（Native 返回）。</summary>
    public ulong SceneTextureId
    {
        get => _sceneTextureId;
        set => SetProperty(ref _sceneTextureId, value);
    }

    /// <summary>RmlUI 叠加纹理 ID。</summary>
    public ulong RmluiTextureId
    {
        get => _rmluiTextureId;
        set => SetProperty(ref _rmluiTextureId, value);
    }

    /// <summary>是否有有效的场景。</summary>
    public bool HasScene => _sceneHandle != 0;
}
