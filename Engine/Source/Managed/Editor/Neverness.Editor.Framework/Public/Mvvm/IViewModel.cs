namespace Neverness.Editor.Framework.Public.Mvvm;

/// <summary>
/// ViewModel 基础接口——纯数据 + 属性变更通知。
/// 所有 ViewModel 必须实现此接口。
/// </summary>
public interface IViewModel
{
    /// <summary>属性变更通知（标准 INotifyPropertyChanged 模式）。</summary>
    event Action<string>? PropertyChanged;

    /// <summary>触发属性变更通知。</summary>
    void OnPropertyChanged(string propertyName);
}
