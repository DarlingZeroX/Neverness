using CommunityToolkit.Mvvm.ComponentModel;
using System.Threading.Tasks;

namespace NevernessLauncher.ViewModels
{
    /// <summary>
    /// 页面 ViewModel 基类
    /// </summary>
    public abstract partial class PageViewModelBase : ViewModelBase
    {
        /// <summary>页面标题</summary>
        [ObservableProperty]
        private string _title = string.Empty;

        /// <summary>页面图标资源键</summary>
        [ObservableProperty]
        private string _iconKey = string.Empty;

        /// <summary>是否为当前选中页面</summary>
        [ObservableProperty]
        private bool _isSelected;

        /// <summary>是否为当前活跃页面</summary>
        [ObservableProperty]
        private bool _isActive;

        /// <summary>页面进入时调用</summary>
        public virtual Task OnNavigatedTo(object? parameter = null) => Task.CompletedTask;

        /// <summary>页面离开时调用</summary>
        public virtual Task OnNavigatedFrom() => Task.CompletedTask;
    }
}
