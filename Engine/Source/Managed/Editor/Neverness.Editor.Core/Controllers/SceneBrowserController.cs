using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Public.Mvvm;
using Neverness.Runtime.Scene;

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
    public void SelectEntity(int entityId, bool addToSelection = false)
    {
        _viewModel.Select(entityId, addToSelection);
    }

    /// <summary>展开/折叠节点。</summary>
    public void ToggleExpand(int entityId)
    {
        _viewModel.SetExpanded(entityId, !_viewModel.IsExpanded(entityId));
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
    public void BeginRename(int entityId)
    {
        // TODO: 弹出重命名对话框（需要 View 层配合）
    }

    /// <summary>重命名实体。</summary>
    public void RenameEntity(IEntity entity, string newName)
    {
        if (_sceneQueryService.RenameEntity(entity, newName))
        {
            RefreshTree();
        }
    }

    /// <summary>删除实体。</summary>
    public void DeleteEntity(IEntity entity)
    {
        if (_sceneQueryService.DeleteEntity(entity))
        {
            if (_viewModel.SelectedEntityId == entity.Id)
                _viewModel.ClearSelection();
            RefreshTree();
        }
    }

    /// <summary>复制实体。</summary>
    public void DuplicateEntity(IEntity entity)
    {
        if (_sceneQueryService.DuplicateEntity(entity) != null)
        {
            RefreshTree();
        }
    }

    /// <summary>拖拽重设父节点。</summary>
    public void ReparentEntity(IEntity entity, IEntity newParent)
    {
        if (_sceneQueryService.ReparentEntity(entity, newParent))
        {
            RefreshTree();
        }
    }

    /// <summary>创建子实体。</summary>
    public void CreateChildEntity(IEntity? parent)
    {
        var newEntity = _sceneQueryService.CreateChildEntity(parent);
        if (newEntity != null)
        {
            RefreshTree();
            _viewModel.Select(newEntity.Id);
        }
    }

    /// <summary>将 EntityNodeData 转换为 EntityNodeVM。</summary>
    private static EntityNodeVM ConvertToVM(EntityNodeData data)
    {
        return new EntityNodeVM
        {
            Id = data.Id,
            Name = data.Name,
            ParentId = data.ParentId,
            Depth = data.Depth,
            Children = data.Children.Select(ConvertToVM).ToList()
        };
    }
}
