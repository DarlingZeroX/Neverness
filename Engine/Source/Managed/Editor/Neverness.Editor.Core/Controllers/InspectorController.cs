using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Framework.Public.Mvvm;

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
    public void SetSelectedEntity(ulong sceneHandle, ulong entityHandle)
    {
        var name = _inspectorService.GetEntityName(sceneHandle, entityHandle);
        _viewModel.SetSelectedEntity(entityHandle, name);
        _viewModel.SceneHandle = sceneHandle;
        _viewModel.IsActive = _inspectorService.IsEntityActive(sceneHandle, entityHandle);
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

        var components = _inspectorService.GetEntityComponents(
            _viewModel.SceneHandle,
            _viewModel.SelectedEntityHandle);

        var vmComponents = components.Select(c => new ComponentInfoVM
        {
            TypeId = c.TypeId,
            DisplayName = c.DisplayName,
            Order = c.Order
        }).ToList();

        _viewModel.UpdateComponents(vmComponents);
    }

    /// <summary>添加组件。</summary>
    public void AddComponent(ulong componentTypeId)
    {
        if (_inspectorService.AddComponent(
            _viewModel.SceneHandle,
            _viewModel.SelectedEntityHandle,
            componentTypeId))
        {
            RefreshComponents();
        }
    }

    /// <summary>移除组件。</summary>
    public void RemoveComponent(ulong componentTypeId)
    {
        if (_inspectorService.RemoveComponent(
            _viewModel.SceneHandle,
            _viewModel.SelectedEntityHandle,
            componentTypeId))
        {
            RefreshComponents();
        }
    }

    /// <summary>设置实体激活状态。</summary>
    public void SetActive(bool active)
    {
        _viewModel.IsActive = active;
        _inspectorService.SetEntityActive(
            _viewModel.SceneHandle,
            _viewModel.SelectedEntityHandle,
            active);
    }

    /// <summary>获取可用的组件类型列表（用于"添加组件"菜单）。</summary>
    public List<ComponentTypeInfo> GetAvailableComponentTypes()
    {
        return _inspectorService.GetAvailableComponentTypes();
    }

    /// <summary>绘制指定组件的 Inspector 字段。</summary>
    public bool DrawComponentInspector(ulong componentTypeId)
    {
        if (!_viewModel.HasSelection) return false;

        return _inspectorService.DrawComponentInspector(
            _viewModel.SceneHandle,
            _viewModel.SelectedEntityHandle,
            componentTypeId);
    }
}
