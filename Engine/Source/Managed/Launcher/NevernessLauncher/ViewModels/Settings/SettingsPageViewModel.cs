using Avalonia;
using Avalonia.Styling;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using NevernessLauncher.Contracts;
using NevernessLauncher.Core.Models;
using System.Collections.ObjectModel;
using System.Linq;
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
        private readonly ILocalizationService _localizationService;

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

        /// <summary>选中的语言对象</summary>
        [ObservableProperty]
        private LanguageInfo? _selectedLanguage;

        /// <summary>启动时自动扫描</summary>
        [ObservableProperty]
        private bool _autoScanOnStartup = true;

        /// <summary>最大最近项目数</summary>
        [ObservableProperty]
        private int _maxRecentProjects = 20;

        /// <summary>主题选项列表</summary>
        public string[] ThemeOptions { get; } = { "Dark", "Light", "System" };

        /// <summary>语言选项列表</summary>
        public ObservableCollection<LanguageInfo> LanguageOptions { get; } = new();

        // 本地化文本属性
        [ObservableProperty] private string _pageTitle = "Settings";
        [ObservableProperty] private string _pageSubtitle = "Configure your launcher preferences";
        [ObservableProperty] private string _appearanceText = "Appearance";
        [ObservableProperty] private string _themeText = "Theme";
        [ObservableProperty] private string _themeDesc = "Choose your preferred color scheme";
        [ObservableProperty] private string _themeDarkText = "Dark";
        [ObservableProperty] private string _themeLightText = "Light";
        [ObservableProperty] private string _themeSystemText = "System";
        [ObservableProperty] private string _languageText = "Language";
        [ObservableProperty] private string _languageDesc = "Select your preferred language";
        [ObservableProperty] private string _generalText = "General";
        [ObservableProperty] private string _autoScanText = "Auto Scan";
        [ObservableProperty] private string _autoScanDesc = "Automatically scan for projects on startup";
        [ObservableProperty] private string _maxRecentText = "Max Recent Projects";
        [ObservableProperty] private string _maxRecentDesc = "Maximum number of recent projects to remember";
        [ObservableProperty] private string _engineText = "Engine";
        [ObservableProperty] private string _defaultEngineText = "Default Engine";
        [ObservableProperty] private string _defaultEngineDesc = "Default engine version for new projects";
        [ObservableProperty] private string _enginePathsText = "Engine Paths";
        [ObservableProperty] private string _enginePathsDesc = "Directories to scan for engine installations";
        [ObservableProperty] private string _addEnginePathText = "Add Engine Path";
        [ObservableProperty] private string _projectsText = "Projects";
        [ObservableProperty] private string _projectDirectoriesText = "Project Directories";
        [ObservableProperty] private string _projectDirectoriesDesc = "Directories to scan for Neverness projects";
        [ObservableProperty] private string _addProjectDirectoryText = "Add Project Directory";
        [ObservableProperty] private string _resetDefaultsText = "Reset to Defaults";
        [ObservableProperty] private string _saveSettingsText = "Save Settings";
        [ObservableProperty] private string _removeText = "Remove";

        public SettingsPageViewModel(
            ILogService logService,
            IConfigurationService configService,
            IEngineService engineService,
            IProjectService projectService,
            ILocalizationService localizationService)
        {
            _logService = logService;
            _configService = configService;
            _engineService = engineService;
            _projectService = projectService;
            _localizationService = localizationService;

            Title = "Settings";
            IconKey = "⚙️";

            // 加载可用语言
            foreach (var lang in _localizationService.AvailableLanguages)
            {
                LanguageOptions.Add(lang);
            }

            // 注册语言变更事件
            _localizationService.LanguageChanged += OnLanguageChanged;

            // 初始化本地化文本
            UpdateLocalizedTexts();
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

        /// <summary>Language 属性变更时触发</summary>
        partial void OnLanguageChanged(string value)
        {
            _localizationService.SetLanguage(value);
        }

        /// <summary>SelectedLanguage 属性变更时触发</summary>
        partial void OnSelectedLanguageChanged(LanguageInfo? value)
        {
            if (value != null)
            {
                Language = value.Code;
            }
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

            // 设置选中的语言
            SelectedLanguage = LanguageOptions.FirstOrDefault(l => l.Code == Language);

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

        private void OnLanguageChanged(object? sender, string language)
        {
            UpdateLocalizedTexts();
        }

        private void UpdateLocalizedTexts()
        {
            PageTitle = _localizationService.GetString("Settings.Title");
            PageSubtitle = _localizationService.GetString("Settings.Subtitle");
            AppearanceText = _localizationService.GetString("Settings.Appearance");
            ThemeText = _localizationService.GetString("Settings.Theme");
            ThemeDesc = _localizationService.GetString("Settings.ThemeDesc");
            ThemeDarkText = _localizationService.GetString("Settings.Theme.Dark");
            ThemeLightText = _localizationService.GetString("Settings.Theme.Light");
            ThemeSystemText = _localizationService.GetString("Settings.Theme.System");
            LanguageText = _localizationService.GetString("Settings.Language");
            LanguageDesc = _localizationService.GetString("Settings.LanguageDesc");
            GeneralText = _localizationService.GetString("Settings.General");
            AutoScanText = _localizationService.GetString("Settings.AutoScan");
            AutoScanDesc = _localizationService.GetString("Settings.AutoScanDesc");
            MaxRecentText = _localizationService.GetString("Settings.MaxRecent");
            MaxRecentDesc = _localizationService.GetString("Settings.MaxRecentDesc");
            EngineText = _localizationService.GetString("Settings.Engine");
            DefaultEngineText = _localizationService.GetString("Settings.DefaultEngine");
            DefaultEngineDesc = _localizationService.GetString("Settings.DefaultEngineDesc");
            EnginePathsText = _localizationService.GetString("Settings.EnginePaths");
            EnginePathsDesc = _localizationService.GetString("Settings.EnginePathsDesc");
            AddEnginePathText = _localizationService.GetString("Settings.AddEnginePath");
            ProjectsText = _localizationService.GetString("Settings.Projects");
            ProjectDirectoriesText = _localizationService.GetString("Settings.ProjectDirectories");
            ProjectDirectoriesDesc = _localizationService.GetString("Settings.ProjectDirectoriesDesc");
            AddProjectDirectoryText = _localizationService.GetString("Settings.AddProjectDirectory");
            ResetDefaultsText = _localizationService.GetString("Settings.ResetDefaults");
            SaveSettingsText = _localizationService.GetString("Settings.SaveSettings");
            RemoveText = _localizationService.GetString("Common.Remove");
        }
    }
}
