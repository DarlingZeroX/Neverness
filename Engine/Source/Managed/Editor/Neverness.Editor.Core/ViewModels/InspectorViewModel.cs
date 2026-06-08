using Neverness.Editor.Framework.Public.Mvvm;

namespace Neverness.Editor.Core.ViewModels;

/// <summary>
/// Inspector ViewModel——暴露选中实体的组件信息供 View 层渲染。
/// </summary>
public class InspectorViewModel : ViewModelBase
{
    // ── 选中状态 ──
    private ulong _selectedEntityHandle;
    private string _selectedEntityName = "";
    private bool _isActive = true;
    private ulong _sceneHandle;

    // ── 组件列表（显示用） ──
    private readonly List<ComponentInfoVM> _components = new();

    // ── 属性 ──

    /// <summary>当前选中的实体句柄。</summary>
    public ulong SelectedEntityHandle
    {
        get => _selectedEntityHandle;
        set => SetProperty(ref _selectedEntityHandle, value);
    }

    /// <summary>当前选中实体的显示名称。</summary>
    public string SelectedEntityName
    {
        get => _selectedEntityName;
        set => SetProperty(ref _selectedEntityName, value);
    }

    /// <summary>实体是否激活。</summary>
    public bool IsActive
    {
        get => _isActive;
        set => SetProperty(ref _isActive, value);
    }

    /// <summary>关联的场景句柄。</summary>
    public ulong SceneHandle
    {
        get => _sceneHandle;
        set => SetProperty(ref _sceneHandle, value);
    }

    /// <summary>是否有选中的实体。</summary>
    public bool HasSelection => _selectedEntityHandle != 0;

    /// <summary>组件信息列表（只读）。</summary>
    public IReadOnlyList<ComponentInfoVM> Components => _components;

    // ── 方法（Controller 调用） ──

    /// <summary>设置选中实体。</summary>
    public void SetSelectedEntity(ulong handle, string name)
    {
        _selectedEntityHandle = handle;
        _selectedEntityName = name;
        OnPropertyChanged(nameof(SelectedEntityHandle));
        OnPropertyChanged(nameof(SelectedEntityName));
        OnPropertyChanged(nameof(HasSelection));
    }

    /// <summary>清空选中。</summary>
    public void ClearSelection()
    {
        _selectedEntityHandle = 0;
        _selectedEntityName = "";
        _components.Clear();
        OnPropertyChanged(nameof(SelectedEntityHandle));
        OnPropertyChanged(nameof(SelectedEntityName));
        OnPropertyChanged(nameof(HasSelection));
        OnPropertyChanged(nameof(Components));
    }

    /// <summary>更新组件列表。</summary>
    public void UpdateComponents(List<ComponentInfoVM> components)
    {
        _components.Clear();
        _components.AddRange(components);
        OnPropertyChanged(nameof(Components));
    }
}

/// <summary>组件信息 ViewModel。</summary>
public class ComponentInfoVM
{
    public ulong TypeId { get; init; }
    public string DisplayName { get; init; } = "";
    public int Order { get; init; }
}
