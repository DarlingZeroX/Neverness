using Avalonia;
using Avalonia.Styling;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using NevernessLauncher.Contracts;
using NevernessLauncher.Core.Models;
using System.Collections.ObjectModel;
using System.Threading.Tasks;

namespace NevernessLauncher.ViewModels.Settings
{
    /// <summary>
    /// 设置页面 ViewModel
    /// </summary>
    public partial class SettingsPageViewModel : PageViewModelBase
    {
        private readonly ILogService _logService;
        private readonly IConfigurationService _configService;
        private readonly IEngineService _engineService;
        private readonly IProjectService _projectService;

        /// <summary>引擎扫描路径</summary>
        public ObservableCollection<string> EnginePaths { get; } = new();

        /// <summary>项目扫描目录</summary>
        public ObservableCollection<string> ProjectDirectories { get; } = new();

        /// <summary>默认引擎版本</summary>
        [ObservableProperty]
        private string? _defaultEngineVersion;

        /// <summary>主题</summary>
        [ObservableProperty]
        private string _theme = "Dark";

        /// <summary>语言</summary>
        [ObservableProperty]
        private string _language = "zh-CN";

        /// <summary>启动时自动扫描</summary>
        [ObservableProperty]
        private bool _autoScanOnStartup = true;

        /// <summary>最大最近项目数</summary>
        [ObservableProperty]
        private int _maxRecentProjects = 20;

        /// <summary>主题选项列表</summary>
        public string[] ThemeOptions { get; } = { "Dark", "Light", "System" };

        public SettingsPageViewModel(
            ILogService logService,
            IConfigurationService configService,
            IEngineService engineService,
            IProjectService projectService)
        {
            _logService = logService;
            _configService = configService;
            _engineService = engineService;
            _projectService = projectService;

            Title = "Settings";
            IconKey = "⚙️";
        }

        /// <summary>页面进入时调用</summary>
        public override async Task OnNavigatedTo(object? parameter = null)
        {
            LoadSettings();
        }

        /// <summary>Theme 属性变更时触发</summary>
        partial void OnThemeChanged(string value)
        {
            ApplyTheme(value);
        }

        /// <summary>应用主题</summary>
        private void ApplyTheme(string theme)
        {
            if (Application.Current == null) return;

            var themeVariant = theme switch
            {
                "Dark" => ThemeVariant.Dark,
                "Light" => ThemeVariant.Light,
                "System" => ThemeVariant.Default,
                _ => ThemeVariant.Dark
            };

            Application.Current.RequestedThemeVariant = themeVariant;
            _logService.LogInfo($"Theme changed to: {theme}");
        }

        /// <summary>保存设置</summary>
        [RelayCommand]
        private async Task Save()
        {
            var settings = new UserSettings
            {
                Theme = Theme,
                Language = Language,
                AutoScanOnStartup = AutoScanOnStartup,
                MaxRecentProjects = MaxRecentProjects,
                DefaultEngineVersion = DefaultEngineVersion,
                EnginePaths = new System.Collections.Generic.List<string>(EnginePaths),
                ProjectDirectories = new System.Collections.Generic.List<string>(ProjectDirectories)
            };

            await _configService.SaveUserSettings(settings);
            _logService.LogInfo("Settings saved");
        }

        /// <summary>恢复默认设置</summary>
        [RelayCommand]
        private void Reset()
        {
            Theme = "Dark";
            Language = "zh-CN";
            AutoScanOnStartup = true;
            MaxRecentProjects = 20;
            DefaultEngineVersion = null;

            EnginePaths.Clear();
            ProjectDirectories.Clear();

            _logService.LogInfo("Settings reset to defaults");
        }

        /// <summary>添加引擎路径</summary>
        [RelayCommand]
        private async Task AddEnginePath()
        {
            // TODO: 实现文件夹选择对话框
            _logService.LogInfo("Add engine path dialog requested");
        }

        /// <summary>移除引擎路径</summary>
        [RelayCommand]
        private async Task RemoveEnginePath(string? path)
        {
            if (!string.IsNullOrEmpty(path))
            {
                EnginePaths.Remove(path);
                await _engineService.RemoveEnginePath(path);
            }
        }

        /// <summary>添加项目目录</summary>
        [RelayCommand]
        private async Task AddProjectDirectory()
        {
            // TODO: 实现文件夹选择对话框
            _logService.LogInfo("Add project directory dialog requested");
        }

        /// <summary>移除项目目录</summary>
        [RelayCommand]
        private async Task RemoveProjectDirectory(string? path)
        {
            if (!string.IsNullOrEmpty(path))
            {
                ProjectDirectories.Remove(path);
                await _projectService.RemoveProjectDirectory(path);
            }
        }

        private void LoadSettings()
        {
            var settings = _configService.LoadUserSettings();

            Theme = settings.Theme;
            Language = settings.Language;
            AutoScanOnStartup = settings.AutoScanOnStartup;
            MaxRecentProjects = settings.MaxRecentProjects;
            DefaultEngineVersion = settings.DefaultEngineVersion;

            EnginePaths.Clear();
            foreach (var path in settings.EnginePaths)
            {
                EnginePaths.Add(path);
            }

            ProjectDirectories.Clear();
            foreach (var path in settings.ProjectDirectories)
            {
                ProjectDirectories.Add(path);
            }
        }
    }
}
