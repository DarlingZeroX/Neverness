using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Public.Mvvm;

namespace Neverness.Editor.Core.Controllers;

/// <summary>
/// 场景浏览器 Controller——处理实体操作。
/// 通过 ISceneQueryService 访问场景数据，不直接依赖 NativeBridge。
/// </summary>
public class SceneBrowserController : IController
{
    private readonly SceneBrowserViewModel _viewModel;
    private readonly ISceneQueryService _sceneQueryService;

    public SceneBrowserController(SceneBrowserViewModel viewModel, ISceneQueryService sceneQueryService)
    {
        _viewModel = viewModel;
        _sceneQueryService = sceneQueryService;
    }

    public void Initialize()
    {
        // 初始刷新
        RefreshTree();
    }

    public void Shutdown()
    {
        // 清理资源
    }

    /// <summary>刷新实体树。</summary>
    public void RefreshTree()
    {
        // 通过 Service 获取层级数据
        if (_sceneQueryService.TryRefreshHierarchy())
        {
            var rootNodes = _sceneQueryService.GetHierarchyTree();
            var allNodes = _sceneQueryService.GetAllNodes();

            // 转换为 ViewModel 数据
            var vmRootNodes = rootNodes.Select(ConvertToVM).ToList();
            var vmAllNodes = allNodes.Select(ConvertToVM).ToList();

            _viewModel.RebuildTree(vmRootNodes, vmAllNodes);
            _viewModel.HasScene = _sceneQueryService.HasActiveScene;
        }
    }

    /// <summary>选中实体。</summary>
    public void SelectEntity(ulong handle, bool addToSelection = false)
    {
        _viewModel.Select(handle, addToSelection);
    }

    /// <summary>展开/折叠节点。</summary>
    public void ToggleExpand(ulong handle)
    {
        _viewModel.SetExpanded(handle, !_viewModel.IsExpanded(handle));
    }

    /// <summary>全部展开。</summary>
    public void ExpandAll() => _viewModel.ExpandAll();

    /// <summary>全部折叠。</summary>
    public void CollapseAll() => _viewModel.CollapseAll();

    /// <summary>设置搜索过滤。</summary>
    public void SetSearchText(string text)
    {
        _viewModel.SearchText = text;
    }

    /// <summary>开始重命名实体。</summary>
    public void BeginRename(ulong handle)
    {
        // TODO: 弹出重命名对话框（需要 View 层配合）
    }

    /// <summary>重命名实体。</summary>
    public void RenameEntity(ulong handle, string newName)
    {
        if (_sceneQueryService.RenameEntity(handle, newName))
        {
            RefreshTree();
        }
    }

    /// <summary>删除实体。</summary>
    public void DeleteEntity(ulong handle)
    {
        if (_sceneQueryService.DeleteEntity(handle))
        {
            if (_viewModel.SelectedEntityHandle == handle)
                _viewModel.ClearSelection();
            RefreshTree();
        }
    }

    /// <summary>复制实体。</summary>
    public void DuplicateEntity(ulong handle)
    {
        if (_sceneQueryService.DuplicateEntity(handle) != 0)
        {
            RefreshTree();
        }
    }

    /// <summary>拖拽重设父节点。</summary>
    public void ReparentEntity(ulong entityHandle, ulong newParentHandle)
    {
        if (_sceneQueryService.ReparentEntity(entityHandle, newParentHandle))
        {
            RefreshTree();
        }
    }

    /// <summary>创建子实体。</summary>
    public void CreateChildEntity(ulong parentHandle)
    {
        var newHandle = _sceneQueryService.CreateChildEntity(parentHandle);
        if (newHandle != 0)
        {
            RefreshTree();
            _viewModel.Select(newHandle);
        }
    }

    /// <summary>将 EntityNodeData 转换为 EntityNodeVM。</summary>
    private static EntityNodeVM ConvertToVM(EntityNodeData data)
    {
        return new EntityNodeVM
        {
            Handle = data.Handle,
            Name = data.Name,
            ParentHandle = data.ParentHandle,
            Depth = data.Depth,
            Children = data.Children.Select(ConvertToVM).ToList()
        };
    }
}
