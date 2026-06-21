using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Public.Mvvm;
using Neverness.Runtime.Scene;

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
        // 初始同步场景状态
        _viewModel.HasScene = _viewportService.HasScene;
    }

    public void Shutdown()
    {
        // 清理资源
    }

    /// <summary>设置关联的场景。</summary>
    public void SetScene(SceneWorld? scene)
    {
        _viewModel.HasScene = scene != null;
        _viewportService.SetScene(scene);
    }

    /// <summary>更新视口尺寸（窗口 resize 时调用）。</summary>
    public void UpdateViewportSize(float width, float height)
    {
        _viewModel.ViewportWidth = width;
        _viewModel.ViewportHeight = height;
    }

    /// <summary>聚焦到指定实体。</summary>
    public void FocusEntity(IEntity entity)
    {
        _viewportService.FocusEntity(entity);
    }

    /// <summary>设置摄像机位置。</summary>
    public void SetCameraPosition(float x, float y, float z)
    {
        _viewportService.SetCameraPosition(x, y, z);
    }

    /// <summary>设置 RmlUI 视口尺寸。</summary>
    public void SetRmlUIViewportSize(uint width, uint height)
    {
        _viewportService.SetRmlUIViewportSize(width, height);
    }

    /// <summary>
    /// 从 ECS 收集渲染命令（SetCamera + DrawSpriteBatch）并序列化为 Flat Buffer。
    /// 返回 null 表示无场景或收集失败。
    /// </summary>
    public byte[]? CollectRenderCommands(float width, float height)
    {
        return _viewportService.CollectRenderCommands(width, height);
    }
}
