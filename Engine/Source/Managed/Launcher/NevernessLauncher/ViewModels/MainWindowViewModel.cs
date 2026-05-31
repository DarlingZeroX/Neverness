using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using NevernessLauncher.Contracts;
using NevernessLauncher.ViewModels.Home;
using NevernessLauncher.ViewModels.Settings;
using System.Collections.ObjectModel;
using System.Threading.Tasks;

namespace NevernessLauncher.ViewModels
{
    /// <summary>
    /// 主窗口 ViewModel
    /// </summary>
    public partial class MainWindowViewModel : ViewModelBase
    {
        private readonly ILogService _logService;
        private readonly IConfigurationService _configService;

        /// <summary>当前活跃页面</summary>
        [ObservableProperty]
        private PageViewModelBase? _currentPage;

        /// <summary>所有注册的页面</summary>
        public ObservableCollection<PageViewModelBase> Pages { get; } = new();

        /// <summary>窗口标题</summary>
        [ObservableProperty]
        private string _title = "Neverness Launcher";

        public MainWindowViewModel(
            ILogService logService,
            IConfigurationService configService,
            HomePageViewModel homePage,
            SettingsPageViewModel settingsPage)
        {
            _logService = logService;
            _configService = configService;

            // 注册页面
            Pages.Add(homePage);
            Pages.Add(settingsPage);
        }

        /// <summary>初始化所有页面</summary>
        public async Task Initialize()
        {
            _logService.LogInfo("MainWindowViewModel initializing...");

            // 导航到默认页面
            if (Pages.Count > 0)
            {
                await NavigateToPage(Pages[0]);
            }
        }

        /// <summary>导航到指定页面</summary>
        public async Task NavigateToPage(PageViewModelBase page)
        {
            if (CurrentPage == page)
                return;

            // 离开当前页面
            if (CurrentPage != null)
            {
                CurrentPage.IsActive = false;
                CurrentPage.IsSelected = false;
                await CurrentPage.OnNavigatedFrom();
            }

            // 进入新页面
            CurrentPage = page;
            CurrentPage.IsActive = true;
            CurrentPage.IsSelected = true;
            await CurrentPage.OnNavigatedTo();

            _logService.LogDebug($"Navigated to page: {page.Title}");
        }

        /// <summary>关闭时清理资源</summary>
        public async Task Shutdown()
        {
            _logService.LogInfo("MainWindowViewModel shutting down...");

            if (CurrentPage != null)
            {
                await CurrentPage.OnNavigatedFrom();
            }

            // 保存当前页面状态
            var settings = _configService.LoadUserSettings();
            if (CurrentPage != null)
            {
                settings.LastActivePage = CurrentPage.Title;
            }
            await _configService.SaveUserSettings(settings);
        }
    }
}
