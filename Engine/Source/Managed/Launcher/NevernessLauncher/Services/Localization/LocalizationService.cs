using NevernessLauncher.Contracts;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text.Json;

namespace NevernessLauncher.Services.Localization
{
    /// <summary>
    /// 本地化服务实现
    /// </summary>
    public class LocalizationService : ILocalizationService
    {
        private readonly ILogService _logService;
        private readonly Dictionary<string, Dictionary<string, string>> _translations = new();
        private string _currentLanguage = "zh-CN";

        /// <summary>当前语言</summary>
        public string CurrentLanguage => _currentLanguage;

        /// <summary>可用语言列表</summary>
        public IReadOnlyList<LanguageInfo> AvailableLanguages { get; } = new List<LanguageInfo>
        {
            new() { Code = "zh-CN", DisplayName = "Chinese", NativeName = "中文" },
            new() { Code = "en-US", DisplayName = "English", NativeName = "English" }
        };

        /// <summary>语言变更事件</summary>
        public event EventHandler<string>? LanguageChanged;

        public LocalizationService(ILogService logService)
        {
            _logService = logService;
            LoadTranslations();
        }

        /// <summary>获取本地化字符串</summary>
        public string GetString(string key)
        {
            if (_translations.TryGetValue(_currentLanguage, out var langDict))
            {
                if (langDict.TryGetValue(key, out var value))
                {
                    return value;
                }
            }

            // 回退到英文
            if (_currentLanguage != "en-US" && _translations.TryGetValue("en-US", out var enDict))
            {
                if (enDict.TryGetValue(key, out var enValue))
                {
                    return enValue;
                }
            }

            // 返回 key 本身
            return key;
        }

        /// <summary>获取本地化字符串（带格式化参数）</summary>
        public string GetString(string key, params object[] args)
        {
            var template = GetString(key);
            try
            {
                return string.Format(template, args);
            }
            catch
            {
                return template;
            }
        }

        /// <summary>切换语言</summary>
        public void SetLanguage(string languageCode)
        {
            if (_currentLanguage != languageCode)
            {
                _currentLanguage = languageCode;
                _logService.LogInfo($"Language changed to: {languageCode}");
                LanguageChanged?.Invoke(this, languageCode);
            }
        }

        private void LoadTranslations()
        {
            // 加载中文翻译
            _translations["zh-CN"] = new Dictionary<string, string>
            {
                // 通用
                ["App.Title"] = "Neverness 启动器",
                ["Common.Save"] = "保存",
                ["Common.Cancel"] = "取消",
                ["Common.Reset"] = "重置",
                ["Common.Delete"] = "删除",
                ["Common.Add"] = "添加",
                ["Common.Remove"] = "移除",
                ["Common.Close"] = "关闭",
                ["Common.Confirm"] = "确认",
                ["Common.Loading"] = "加载中...",
                ["Common.Error"] = "错误",
                ["Common.Success"] = "成功",

                // 导航
                ["Nav.Home"] = "首页",
                ["Nav.Settings"] = "设置",

                // 首页
                ["Home.Title"] = "项目",
                ["Home.Subtitle"] = "管理和启动你的 Neverness 项目",
                ["Home.Scan"] = "扫描",
                ["Home.Open"] = "打开",
                ["Home.New"] = "新建",
                ["Home.Pinned"] = "已置顶",
                ["Home.Recent"] = "最近",
                ["Home.NoProjects"] = "未找到项目",
                ["Home.NoProjectsDesc"] = "扫描项目或创建新项目",
                ["Home.CreateNew"] = "创建新项目",
                ["Home.Editor"] = "编辑器",
                ["Home.Play"] = "运行",
                ["Home.SearchPlaceholder"] = "搜索项目...",
                ["Home.Scanning"] = "正在扫描项目...",

                // 设置
                ["Settings.Title"] = "设置",
                ["Settings.Subtitle"] = "配置启动器偏好",
                ["Settings.Appearance"] = "外观",
                ["Settings.Theme"] = "主题",
                ["Settings.ThemeDesc"] = "选择你喜欢的配色方案",
                ["Settings.Theme.Dark"] = "深色",
                ["Settings.Theme.Light"] = "浅色",
                ["Settings.Theme.System"] = "跟随系统",
                ["Settings.Language"] = "语言",
                ["Settings.LanguageDesc"] = "选择界面语言",
                ["Settings.General"] = "通用",
                ["Settings.AutoScan"] = "自动扫描",
                ["Settings.AutoScanDesc"] = "启动时自动扫描项目",
                ["Settings.MaxRecent"] = "最大最近项目数",
                ["Settings.MaxRecentDesc"] = "记住的最近项目最大数量",
                ["Settings.Engine"] = "引擎",
                ["Settings.DefaultEngine"] = "默认引擎",
                ["Settings.DefaultEngineDesc"] = "新项目的默认引擎版本",
                ["Settings.EnginePaths"] = "引擎路径",
                ["Settings.EnginePathsDesc"] = "扫描引擎安装的目录",
                ["Settings.AddEnginePath"] = "添加引擎路径",
                ["Settings.Projects"] = "项目",
                ["Settings.ProjectDirectories"] = "项目目录",
                ["Settings.ProjectDirectoriesDesc"] = "扫描 Neverness 项目的目录",
                ["Settings.AddProjectDirectory"] = "添加项目目录",
                ["Settings.ResetDefaults"] = "恢复默认",
                ["Settings.SaveSettings"] = "保存设置",

                // 状态
                ["Status.Ready"] = "引擎就绪",
                ["Status.Error"] = "引擎错误",
                ["Status.Scanning"] = "扫描中...",

                // 项目状态
                ["ProjectStatus.Valid"] = "有效",
                ["ProjectStatus.EngineMissing"] = "引擎缺失",
                ["ProjectStatus.InvalidPath"] = "路径无效",
                ["ProjectStatus.Corrupted"] = "配置损坏",
                ["ProjectStatus.VersionMismatch"] = "版本不匹配"
            };

            // 加载英文翻译
            _translations["en-US"] = new Dictionary<string, string>
            {
                // Common
                ["App.Title"] = "Neverness Launcher",
                ["Common.Save"] = "Save",
                ["Common.Cancel"] = "Cancel",
                ["Common.Reset"] = "Reset",
                ["Common.Delete"] = "Delete",
                ["Common.Add"] = "Add",
                ["Common.Remove"] = "Remove",
                ["Common.Close"] = "Close",
                ["Common.Confirm"] = "Confirm",
                ["Common.Loading"] = "Loading...",
                ["Common.Error"] = "Error",
                ["Common.Success"] = "Success",

                // Navigation
                ["Nav.Home"] = "Home",
                ["Nav.Settings"] = "Settings",

                // Home
                ["Home.Title"] = "Projects",
                ["Home.Subtitle"] = "Manage and launch your Neverness projects",
                ["Home.Scan"] = "Scan",
                ["Home.Open"] = "Open",
                ["Home.New"] = "New",
                ["Home.Pinned"] = "Pinned",
                ["Home.Recent"] = "Recent",
                ["Home.NoProjects"] = "No projects found",
                ["Home.NoProjectsDesc"] = "Scan for projects or create a new one",
                ["Home.CreateNew"] = "Create New Project",
                ["Home.Editor"] = "Editor",
                ["Home.Play"] = "Play",
                ["Home.SearchPlaceholder"] = "Search projects...",
                ["Home.Scanning"] = "Scanning for projects...",

                // Settings
                ["Settings.Title"] = "Settings",
                ["Settings.Subtitle"] = "Configure your launcher preferences",
                ["Settings.Appearance"] = "Appearance",
                ["Settings.Theme"] = "Theme",
                ["Settings.ThemeDesc"] = "Choose your preferred color scheme",
                ["Settings.Theme.Dark"] = "Dark",
                ["Settings.Theme.Light"] = "Light",
                ["Settings.Theme.System"] = "System",
                ["Settings.Language"] = "Language",
                ["Settings.LanguageDesc"] = "Select your preferred language",
                ["Settings.General"] = "General",
                ["Settings.AutoScan"] = "Auto Scan",
                ["Settings.AutoScanDesc"] = "Automatically scan for projects on startup",
                ["Settings.MaxRecent"] = "Max Recent Projects",
                ["Settings.MaxRecentDesc"] = "Maximum number of recent projects to remember",
                ["Settings.Engine"] = "Engine",
                ["Settings.DefaultEngine"] = "Default Engine",
                ["Settings.DefaultEngineDesc"] = "Default engine version for new projects",
                ["Settings.EnginePaths"] = "Engine Paths",
                ["Settings.EnginePathsDesc"] = "Directories to scan for engine installations",
                ["Settings.AddEnginePath"] = "Add Engine Path",
                ["Settings.Projects"] = "Projects",
                ["Settings.ProjectDirectories"] = "Project Directories",
                ["Settings.ProjectDirectoriesDesc"] = "Directories to scan for Neverness projects",
                ["Settings.AddProjectDirectory"] = "Add Project Directory",
                ["Settings.ResetDefaults"] = "Reset to Defaults",
                ["Settings.SaveSettings"] = "Save Settings",

                // Status
                ["Status.Ready"] = "Engine Ready",
                ["Status.Error"] = "Engine Error",
                ["Status.Scanning"] = "Scanning...",

                // Project Status
                ["ProjectStatus.Valid"] = "Valid",
                ["ProjectStatus.EngineMissing"] = "Engine Missing",
                ["ProjectStatus.InvalidPath"] = "Invalid Path",
                ["ProjectStatus.Corrupted"] = "Config Corrupted",
                ["ProjectStatus.VersionMismatch"] = "Version Mismatch"
            };

            _logService.LogInfo($"Loaded translations for {_translations.Count} languages");
        }
    }
}
