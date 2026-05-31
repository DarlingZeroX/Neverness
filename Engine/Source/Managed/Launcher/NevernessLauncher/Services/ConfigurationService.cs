using NevernessLauncher.Contracts;
using NevernessLauncher.Core.Constants;
using NevernessLauncher.Core.Models;
using System;
using System.IO;
using System.Text.Json;
using System.Threading.Tasks;

namespace NevernessLauncher.Services
{
    /// <summary>
    /// 配置服务实现
    /// </summary>
    public class ConfigurationService : IConfigurationService
    {
        private readonly ILogService _logService;
        private readonly string _dataDirectory;

        public ConfigurationService(ILogService logService)
        {
            _logService = logService;
            _dataDirectory = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                PathConstants.ConfigDirectoryName,
                PathConstants.LauncherDataDirectoryName
            );

            if (!Directory.Exists(_dataDirectory))
            {
                Directory.CreateDirectory(_dataDirectory);
            }
        }

        /// <summary>加载 Launcher 应用配置</summary>
        public LauncherConfig LoadLauncherConfig()
        {
            var installPath = AppDomain.CurrentDomain.BaseDirectory;
            var config = new LauncherConfig
            {
                InstallPath = installPath,
                Version = new Version(AppConstants.AppVersion),
                DataDirectory = _dataDirectory
            };

            _logService.LogInfo($"Launcher config loaded: InstallPath={installPath}");
            return config;
        }

        /// <summary>加载用户设置</summary>
        public UserSettings LoadUserSettings()
        {
            var settingsPath = GetUserSettingsPath();
            if (!File.Exists(settingsPath))
            {
                _logService.LogInfo("User settings file not found, using defaults");
                return new UserSettings();
            }

            try
            {
                var json = File.ReadAllText(settingsPath);
                var settings = JsonSerializer.Deserialize<UserSettings>(json);
                _logService.LogInfo("User settings loaded successfully");
                return settings ?? new UserSettings();
            }
            catch (Exception ex)
            {
                _logService.LogError("Failed to load user settings, using defaults", ex);
                return new UserSettings();
            }
        }

        /// <summary>保存用户设置</summary>
        public async Task SaveUserSettings(UserSettings settings)
        {
            var settingsPath = GetUserSettingsPath();
            try
            {
                var options = new JsonSerializerOptions { WriteIndented = true };
                var json = JsonSerializer.Serialize(settings, options);
                await File.WriteAllTextAsync(settingsPath, json);
                _logService.LogInfo("User settings saved successfully");
            }
            catch (Exception ex)
            {
                _logService.LogError("Failed to save user settings", ex);
                throw;
            }
        }

        /// <summary>加载最近项目缓存</summary>
        public RecentProjectsCache LoadRecentProjects()
        {
            var cachePath = GetRecentProjectsPath();
            if (!File.Exists(cachePath))
            {
                _logService.LogInfo("Recent projects cache not found, creating new");
                return new RecentProjectsCache();
            }

            try
            {
                var json = File.ReadAllText(cachePath);
                var cache = JsonSerializer.Deserialize<RecentProjectsCache>(json);
                _logService.LogInfo($"Recent projects loaded: {cache?.Projects.Count ?? 0} projects");
                return cache ?? new RecentProjectsCache();
            }
            catch (Exception ex)
            {
                _logService.LogError("Failed to load recent projects, creating new cache", ex);
                return new RecentProjectsCache();
            }
        }

        /// <summary>保存最近项目缓存</summary>
        public async Task SaveRecentProjects(RecentProjectsCache cache)
        {
            var cachePath = GetRecentProjectsPath();
            try
            {
                var options = new JsonSerializerOptions { WriteIndented = true };
                var json = JsonSerializer.Serialize(cache, options);
                await File.WriteAllTextAsync(cachePath, json);
                _logService.LogInfo("Recent projects saved successfully");
            }
            catch (Exception ex)
            {
                _logService.LogError("Failed to save recent projects", ex);
                throw;
            }
        }

        /// <summary>获取数据存储目录</summary>
        public string GetDataDirectory() => _dataDirectory;

        /// <summary>获取用户设置文件路径</summary>
        public string GetUserSettingsPath() => Path.Combine(_dataDirectory, PathConstants.SettingsFileName);

        /// <summary>获取最近项目文件路径</summary>
        public string GetRecentProjectsPath() => Path.Combine(_dataDirectory, PathConstants.RecentProjectsFileName);
    }
}
