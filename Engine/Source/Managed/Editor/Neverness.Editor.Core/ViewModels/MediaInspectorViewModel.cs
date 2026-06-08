using Neverness.Editor.Framework.Public.Mvvm;

namespace Neverness.Editor.Core.ViewModels;

/// <summary>
/// 媒体 Inspector ViewModel——通用组件 Inspector 状态。
/// 用于 AudioSource、VideoPlayer 等媒体组件 Inspector。
/// </summary>
public class MediaInspectorViewModel : ViewModelBase
{
    private ulong _entityHandle;
    private ulong _componentTypeId;
    private bool _isModified;

    /// <summary>关联的实体句柄。</summary>
    public ulong EntityHandle
    {
        get => _entityHandle;
        set => SetProperty(ref _entityHandle, value);
    }

    /// <summary>组件类型 ID。</summary>
    public ulong ComponentTypeId
    {
        get => _componentTypeId;
        set => SetProperty(ref _componentTypeId, value);
    }

    /// <summary>组件数据是否已修改。</summary>
    public bool IsModified
    {
        get => _isModified;
        set => SetProperty(ref _isModified, value);
    }

    /// <summary>标记已修改。</summary>
    public void MarkModified()
    {
        _isModified = true;
        OnPropertyChanged(nameof(IsModified));
    }

    /// <summary>清除修改标记。</summary>
    public void ClearModified()
    {
        _isModified = false;
        OnPropertyChanged(nameof(IsModified));
    }
}
