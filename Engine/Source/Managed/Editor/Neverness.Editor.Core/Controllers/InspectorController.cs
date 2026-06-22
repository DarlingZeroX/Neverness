using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Public.Mvvm;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Core.Controllers;

/// <summary>
/// Inspector Controller——处理组件操作。
/// 通过 IInspectorService 访问组件数据，不直接依赖 ComponentInspectorRegistry。
/// </summary>
public class InspectorController : IController
{
    private readonly InspectorViewModel _viewModel;
    private readonly IInspectorService _inspectorService;

    public InspectorController(InspectorViewModel viewModel, IInspectorService inspectorService)
    {
        _viewModel = viewModel;
        _inspectorService = inspectorService;
    }

    public void Initialize()
    {
        // 订阅 SelectionChanged 事件（通过 EventBus）
    }

    public void Shutdown()
    {
        // 取消订阅
    }

    /// <summary>设置选中实体。</summary>
    public void SetSelectedEntity(IEntity entity)
    {
        if (entity == null || !entity.IsValid)
        {
            ClearSelection();
            return;
        }

        var name = _inspectorService.GetEntityName(entity);
        _viewModel.SetSelectedEntity(entity.Id, name);
        _viewModel.IsActive = _inspectorService.IsEntityActive(entity);
        RefreshComponents();
    }

    /// <summary>清空选中。</summary>
    public void ClearSelection()
    {
        _viewModel.ClearSelection();
    }

    /// <summary>刷新组件列表。</summary>
    public void RefreshComponents()
    {
        if (!_viewModel.HasSelection) return;

        // 通过 entityId 获取 IEntity 实例
        var entity = _inspectorService.GetEntityById(_viewModel.SelectedEntityId);
        if (entity == null || !entity.IsValid)
        {
            _viewModel.UpdateComponents(new List<ComponentInfoVM>());
            return;
        }

        // 从 Service 获取组件列表
        var components = _inspectorService.GetEntityComponents(entity);

        var vmComponents = components.Select(c => new ComponentInfoVM
        {
            TypeId = c.TypeId,
            DisplayName = c.DisplayName,
            Order = c.Order
        }).ToList();

        _viewModel.UpdateComponents(vmComponents);
    }

    /// <summary>添加组件。</summary>
    public void AddComponent(IEntity entity, ulong componentTypeId)
    {
        if (_inspectorService.AddComponent(entity, componentTypeId))
        {
            RefreshComponents();
        }
    }

    /// <summary>添加组件（便捷重载——自动通过 ViewModel 中的 EntityId 查找实体）。</summary>
    public void AddComponent(ulong componentTypeId)
    {
        if (!_viewModel.HasSelection) return;

        var entity = _inspectorService.GetEntityById(_viewModel.SelectedEntityId);
        if (entity == null || !entity.IsValid) return;

        AddComponent(entity, componentTypeId);
    }

    /// <summary>移除组件。</summary>
    public void RemoveComponent(IEntity entity, ulong componentTypeId)
    {
        if (_inspectorService.RemoveComponent(entity, componentTypeId))
        {
            RefreshComponents();
        }
    }

    /// <summary>设置实体激活状态。</summary>
    public void SetActive(IEntity entity, bool active)
    {
        _viewModel.IsActive = active;
        _inspectorService.SetEntityActive(entity, active);
    }

    /// <summary>获取可用的组件类型列表（用于"添加组件"菜单）。</summary>
    public List<ComponentTypeInfo> GetAvailableComponentTypes()
    {
        return _inspectorService.GetAvailableComponentTypes();
    }

    /// <summary>绘制指定组件的 Inspector 字段。</summary>
    public bool DrawComponentInspector(IEntity entity, ulong componentTypeId)
    {
        if (!_viewModel.HasSelection) return false;

        return _inspectorService.DrawComponentInspector(entity, componentTypeId);
    }
}
