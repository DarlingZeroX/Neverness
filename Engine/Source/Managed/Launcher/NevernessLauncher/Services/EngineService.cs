using NevernessLauncher.Contracts;
using NevernessLauncher.Core.Constants;
using NevernessLauncher.Core.Enums;
using NevernessLauncher.Core.Models;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Threading.Tasks;

namespace NevernessLauncher.Services
{
    /// <summary>
    /// 引擎服务实现
    /// </summary>
    public class EngineService : IEngineService
    {
        private readonly ILogService _logService;
        private readonly IConfigurationService _configService;
        private readonly List<EngineVersion> _engines = new();

        public event EventHandler? EnginesChanged;

        public EngineService(ILogService logService, IConfigurationService configService)
        {
            _logService = logService;
            _configService = configService;
        }

        /// <summary>扫描本地引擎安装</summary>
        public async Task<IReadOnlyList<EngineVersion>> ScanEngines()
        {
            _logService.LogInfo("Starting engine scan...");
            _engines.Clear();

            // 1. 扫描环境变量
            ScanFromEnvironmentVariable();

            // 2. 扫描固定路径
            ScanFromFixedPaths();

            // 3. 扫描用户自定义路径
            ScanFromUserPaths();

            // 去重并验证
            var validEngines = _engines
                .Where(e => ValidateEngine(e))
                .GroupBy(e => e.Version.ToString())
                .Select(g => g.First())
                .OrderByDescending(e => e.Version)
                .ToList();

            _engines.Clear();
            _engines.AddRange(validEngines);

            _logService.LogInfo($"Engine scan completed: {_engines.Count} engines found");
            EnginesChanged?.Invoke(this, EventArgs.Empty);

            return _engines.AsReadOnly();
        }

        /// <summary>获取已安装的引擎列表</summary>
        public IReadOnlyList<EngineVersion> GetInstalledEngines() => _engines.AsReadOnly();

        /// <summary>查找指定版本的引擎</summary>
        public EngineVersion? FindEngine(Version version)
        {
            return _engines.FirstOrDefault(e => e.Version == version);
        }

        /// <summary>查找兼容的引擎版本（最低版本要求）</summary>
        public EngineVersion? FindCompatibleEngine(Version minimumVersion)
        {
            return _engines
                .Where(e => e.Version >= minimumVersion && e.Channel == EngineChannel.Stable)
                .OrderByDescending(e => e.Version)
                .FirstOrDefault();
        }

        /// <summary>获取默认引擎版本</summary>
        public EngineVersion? GetDefaultEngine()
        {
            return _engines.FirstOrDefault(e => e.IsDefault) ?? _engines.FirstOrDefault();
        }

        /// <summary>设置默认引擎版本</summary>
        public Task SetDefaultEngine(Version version)
        {
            foreach (var engine in _engines)
            {
                engine.IsDefault = engine.Version == version;
            }
            _logService.LogInfo($"Default engine set to: {version}");
            return Task.CompletedTask;
        }

        /// <summary>验证引擎安装是否有效</summary>
        public bool ValidateEngine(EngineVersion engine)
        {
            if (string.IsNullOrEmpty(engine.InstallPath))
                return false;

            if (!Directory.Exists(engine.InstallPath))
                return false;

            // 检查 Editor 可执行文件
            var editorPath = Path.Combine(engine.InstallPath, PathConstants.EditorRelativePath);
            engine.EditorPath = editorPath;

            // 检查 Runtime 可执行文件
            var runtimePath = Path.Combine(engine.InstallPath, PathConstants.RuntimeRelativePath);
            engine.RuntimePath = runtimePath;

            engine.IsValid = File.Exists(editorPath) || File.Exists(runtimePath);
            return engine.IsValid;
        }

        /// <summary>添加自定义引擎扫描路径</summary>
        public async Task AddEnginePath(string path)
        {
            var settings = _configService.LoadUserSettings();
            if (!settings.EnginePaths.Contains(path))
            {
                settings.EnginePaths.Add(path);
                await _configService.SaveUserSettings(settings);
                _logService.LogInfo($"Added engine path: {path}");
            }
        }

        /// <summary>移除自定义引擎扫描路径</summary>
        public async Task RemoveEnginePath(string path)
        {
            var settings = _configService.LoadUserSettings();
            if (settings.EnginePaths.Remove(path))
            {
                await _configService.SaveUserSettings(settings);
                _logService.LogInfo($"Removed engine path: {path}");
            }
        }

        /// <summary>获取所有引擎扫描路径</summary>
        public IReadOnlyList<string> GetEnginePaths()
        {
            var settings = _configService.LoadUserSettings();
            return settings.EnginePaths.AsReadOnly();
        }

        private void ScanFromEnvironmentVariable()
        {
            var envPath = Environment.GetEnvironmentVariable(PathConstants.EnvEngineRoot);
            if (!string.IsNullOrEmpty(envPath) && Directory.Exists(envPath))
            {
                TryAddEngine(envPath, EngineSource.EnvironmentVariable);
            }
        }

        private void ScanFromFixedPaths()
        {
            foreach (var path in PathConstants.FixedEnginePaths)
            {
                if (Directory.Exists(path))
                {
                    // 扫描子目录（版本目录）
                    foreach (var subDir in Directory.GetDirectories(path))
                    {
                        TryAddEngine(subDir, EngineSource.FixedPath);
                    }
                }
            }
        }

        private void ScanFromUserPaths()
        {
            var settings = _configService.LoadUserSettings();
            foreach (var path in settings.EnginePaths)
            {
                if (Directory.Exists(path))
                {
                    TryAddEngine(path, EngineSource.UserDefined);
                }
            }
        }

        private void TryAddEngine(string path, EngineSource source)
        {
            try
            {
                var versionFile = Path.Combine(path, AppConstants.EngineVersionFile);
                if (File.Exists(versionFile))
                {
                    var json = File.ReadAllText(versionFile);
                    var versionInfo = JsonSerializer.Deserialize<JsonElement>(json);

                    if (versionInfo.TryGetProperty("version", out var versionProp))
                    {
                        var versionStr = versionProp.GetString();
                        if (Version.TryParse(versionStr, out var version))
                        {
                            var channel = EngineChannel.Stable;
                            if (versionInfo.TryGetProperty("channel", out var channelProp))
                            {
                                Enum.TryParse(channelProp.GetString(), true, out channel);
                            }

                            _engines.Add(new EngineVersion
                            {
                                Version = version,
                                Channel = channel,
                                InstallPath = path,
                                Source = source,
                                DiscoveredTime = DateTimeOffset.Now
                            });

                            _logService.LogDebug($"Found engine: {version} at {path}");
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                _logService.LogWarning($"Failed to read engine info from: {path} - {ex.Message}");
            }
        }
    }
}
