using System.Runtime.CompilerServices;

namespace Neverness.Editor.Framework.Public.Mvvm;

/// <summary>
/// ViewModel 基类——提供属性变更通知的通用实现。
/// </summary>
public abstract class ViewModelBase : IViewModel
{
    public event Action<string>? PropertyChanged;

    public void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(propertyName ?? string.Empty);
    }

    /// <summary>设置属性值并触发变更通知。</summary>
    protected bool SetProperty<T>(ref T field, T value, [CallerMemberName] string? propertyName = null)
    {
        if (EqualityComparer<T>.Default.Equals(field, value))
            return false;

        field = value;
        OnPropertyChanged(propertyName);
        return true;
    }
}
