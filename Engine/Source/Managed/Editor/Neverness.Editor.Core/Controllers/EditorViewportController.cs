using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Public.Mvvm;

namespace Neverness.Editor.Core.Controllers;

/// <summary>
/// 编辑器视口 Controller——处理视口操作。
/// 通过 IViewportService 访问渲染数据，不直接依赖 Native API。
/// </summary>
public class EditorViewportController : IController
{
    private readonly EditorViewportViewModel _viewModel;
    private readonly IViewportService _viewportService;

    public EditorViewportController(EditorViewportViewModel viewModel, IViewportService viewportService)
    {
        _viewModel = viewModel;
        _viewportService = viewportService;
    }

    public void Initialize()
    {
        // 初始同步场景句柄
        _viewModel.SceneHandle = _viewportService.SceneHandle;
    }

    public void Shutdown()
    {
        // 清理资源
    }

    /// <summary>设置关联的场景。</summary>
    public void SetScene(ulong sceneHandle)
    {
        _viewModel.SceneHandle = sceneHandle;
        _viewportService.SetScene(sceneHandle);
    }

    /// <summary>更新视口尺寸（窗口 resize 时调用）。</summary>
    public void UpdateViewportSize(float width, float height)
    {
        _viewModel.ViewportWidth = width;
        _viewModel.ViewportHeight = height;
    }

    /// <summary>调用渲染服务并更新纹理 ID。</summary>
    public void RenderScene()
    {
        if (!_viewModel.HasScene) return;

        var width = (uint)_viewModel.ViewportWidth;
        var height = (uint)_viewModel.ViewportHeight;

        if (width < 1 || height < 1) return;

        // 调用 Service 渲染场景
        var textureId = _viewportService.RenderSceneToTexture(width, height);
        _viewModel.SceneTextureId = textureId;

        // 获取 RmlUI 纹理
        _viewModel.RmluiTextureId = _viewportService.GetLastRmluiTextureId();

        if (textureId == 0)
        {
            Console.WriteLine($"[ViewportController] RenderScene: textureId=0 (w={width}, h={height}, sceneHandle={_viewModel.SceneHandle})");
        }
    }

    /// <summary>聚焦到指定实体。</summary>
    public void FocusEntity(ulong entityHandle)
    {
        _viewportService.FocusEntity(entityHandle);
    }

    /// <summary>设置摄像机位置。</summary>
    public void SetCameraPosition(float x, float y, float z)
    {
        _viewportService.SetCameraPosition(x, y, z);
    }

    /// <summary>获取渲染统计。</summary>
    public RenderStats GetRenderStats()
    {
        return _viewportService.GetRenderStats();
    }

    /// <summary>设置 RmlUI 视口尺寸。</summary>
    public void SetRmlUIViewportSize(uint width, uint height)
    {
        _viewportService.SetRmlUIViewportSize(width, height);
    }
}
