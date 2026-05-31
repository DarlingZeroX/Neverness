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
    /// 项目服务实现
    /// </summary>
    public class ProjectService : IProjectService
    {
        private readonly ILogService _logService;
        private readonly IConfigurationService _configService;
        private readonly IRecentProjectService _recentProjectService;
        private readonly List<ProjectInfo> _projects = new();

        public event EventHandler? ProjectsChanged;

        public ProjectService(
            ILogService logService,
            IConfigurationService configService,
            IRecentProjectService recentProjectService)
        {
            _logService = logService;
            _configService = configService;
            _recentProjectService = recentProjectService;
        }

        /// <summary>扫描项目</summary>
        public async Task<IReadOnlyList<ProjectInfo>> ScanProjects()
        {
            _logService.LogInfo("Starting project scan...");
            _projects.Clear();

            // 1. 从最近项目列表加载
            LoadFromRecentProjects();

            // 2. 从配置的项目目录扫描
            await ScanFromProjectDirectories();

            // 去重并验证
            var validProjects = _projects
                .GroupBy(p => p.ProjectFilePath)
                .Select(g => g.First())
                .ToList();

            _projects.Clear();
            _projects.AddRange(validProjects);

            _logService.LogInfo($"Project scan completed: {_projects.Count} projects found");
            ProjectsChanged?.Invoke(this, EventArgs.Empty);

            return _projects.AsReadOnly();
        }

        /// <summary>获取所有项目列表</summary>
        public IReadOnlyList<ProjectInfo> GetProjects() => _projects.AsReadOnly();

        /// <summary>从 .nnproject 文件加载项目信息</summary>
        public ProjectInfo? LoadProject(string projectFilePath)
        {
            try
            {
                if (!File.Exists(projectFilePath))
                {
                    _logService.LogWarning($"Project file not found: {projectFilePath}");
                    return null;
                }

                var json = File.ReadAllText(projectFilePath);
                var projectData = JsonSerializer.Deserialize<JsonElement>(json);

                var project = new ProjectInfo
                {
                    ProjectFilePath = projectFilePath,
                    ProjectPath = Path.GetDirectoryName(projectFilePath) ?? string.Empty
                };

                // 解析项目信息
                if (projectData.TryGetProperty("project", out var projectInfo))
                {
                    if (projectInfo.TryGetProperty("name", out var nameProp))
                        project.Name = nameProp.GetString() ?? string.Empty;
                    if (projectInfo.TryGetProperty("id", out var idProp) && Guid.TryParse(idProp.GetString(), out var id))
                        project.Id = id;
                    if (projectInfo.TryGetProperty("description", out var descProp))
                        project.Description = descProp.GetString() ?? string.Empty;
                    if (projectInfo.TryGetProperty("author", out var authorProp))
                        project.Author = authorProp.GetString() ?? string.Empty;
                    if (projectInfo.TryGetProperty("created", out var createdProp) && DateTimeOffset.TryParse(createdProp.GetString(), out var created))
                        project.CreatedTime = created;
                    if (projectInfo.TryGetProperty("modified", out var modifiedProp) && DateTimeOffset.TryParse(modifiedProp.GetString(), out var modified))
                        project.ModifiedTime = modified;
                }

                // 解析引擎信息
                if (projectData.TryGetProperty("engine", out var engineInfo))
                {
                    if (engineInfo.TryGetProperty("version", out var versionProp))
                        project.EngineVersion = versionProp.GetString() ?? string.Empty;
                    if (engineInfo.TryGetProperty("minimumVersion", out var minVersionProp))
                        project.MinimumEngineVersion = minVersionProp.GetString() ?? string.Empty;
                    if (engineInfo.TryGetProperty("channel", out var channelProp) && Enum.TryParse(channelProp.GetString(), true, out EngineChannel channel))
                        project.EngineChannel = channel;
                }

                // 解析路径
                if (projectData.TryGetProperty("paths", out var pathsInfo))
                {
                    if (pathsInfo.TryGetProperty("assets", out var assetsProp))
                        project.AssetsPath = assetsProp.GetString() ?? "Assets";
                    if (pathsInfo.TryGetProperty("scenes", out var scenesProp))
                        project.ScenesPath = scenesProp.GetString() ?? "Scenes";
                    if (pathsInfo.TryGetProperty("config", out var configProp))
                        project.ConfigPath = configProp.GetString() ?? "Config";
                    if (pathsInfo.TryGetProperty("build", out var buildProp))
                        project.BuildPath = buildProp.GetString() ?? "Build";
                }

                // 解析设置
                if (projectData.TryGetProperty("settings", out var settingsInfo))
                {
                    if (settingsInfo.TryGetProperty("defaultScene", out var sceneProp))
                        project.DefaultScene = sceneProp.GetString();
                    if (settingsInfo.TryGetProperty("targetPlatform", out var platformProp))
                        project.TargetPlatform = platformProp.GetString() ?? "Windows";
                }

                _logService.LogDebug($"Loaded project: {project.Name} from {projectFilePath}");
                return project;
            }
            catch (Exception ex)
            {
                _logService.LogError($"Failed to load project: {projectFilePath}", ex);
                return null;
            }
        }

        /// <summary>验证项目是否有效</summary>
        public bool ValidateProject(ProjectInfo project)
        {
            if (string.IsNullOrEmpty(project.ProjectPath))
            {
                project.Status = ProjectStatus.InvalidPath;
                return false;
            }

            if (!Directory.Exists(project.ProjectPath))
            {
                project.Status = ProjectStatus.InvalidPath;
                return false;
            }

            if (!File.Exists(project.ProjectFilePath))
            {
                project.Status = ProjectStatus.Corrupted;
                return false;
            }

            project.Status = ProjectStatus.Valid;
            return true;
        }

        /// <summary>打开项目文件（通过文件对话框）</summary>
        public async Task<ProjectInfo?> OpenProjectFile()
        {
            // 注意：这里需要在 UI 层实现文件对话框
            // 这里只是预留接口
            _logService.LogInfo("Open project file dialog requested");
            return null;
        }

        /// <summary>添加项目扫描目录</summary>
        public async Task AddProjectDirectory(string directory)
        {
            var settings = _configService.LoadUserSettings();
            if (!settings.ProjectDirectories.Contains(directory))
            {
                settings.ProjectDirectories.Add(directory);
                await _configService.SaveUserSettings(settings);
                _logService.LogInfo($"Added project directory: {directory}");
            }
        }

        /// <summary>移除项目扫描目录</summary>
        public async Task RemoveProjectDirectory(string directory)
        {
            var settings = _configService.LoadUserSettings();
            if (settings.ProjectDirectories.Remove(directory))
            {
                await _configService.SaveUserSettings(settings);
                _logService.LogInfo($"Removed project directory: {directory}");
            }
        }

        /// <summary>获取项目扫描目录列表</summary>
        public IReadOnlyList<string> GetProjectDirectories()
        {
            var settings = _configService.LoadUserSettings();
            return settings.ProjectDirectories.AsReadOnly();
        }

        private void LoadFromRecentProjects()
        {
            var recentProjects = _recentProjectService.GetRecentProjects();
            foreach (var recent in recentProjects)
            {
                if (File.Exists(recent.ProjectPath))
                {
                    var project = LoadProject(recent.ProjectPath);
                    if (project != null)
                    {
                        project.LastOpenedTime = recent.LastAccessTime;
                        _projects.Add(project);
                    }
                }
            }
        }

        private async Task ScanFromProjectDirectories()
        {
            var settings = _configService.LoadUserSettings();
            foreach (var directory in settings.ProjectDirectories)
            {
                if (!Directory.Exists(directory))
                    continue;

                await ScanDirectoryForProjects(directory);
            }
        }

        private async Task ScanDirectoryForProjects(string directory)
        {
            try
            {
                // 查找 .nnproject 文件
                var projectFiles = Directory.GetFiles(directory, $"*{AppConstants.ProjectFileExtension}", SearchOption.AllDirectories);

                foreach (var projectFile in projectFiles)
                {
                    var project = LoadProject(projectFile);
                    if (project != null && !_projects.Any(p => p.ProjectFilePath == projectFile))
                    {
                        _projects.Add(project);
                    }
                }
            }
            catch (Exception ex)
            {
                _logService.LogError($"Failed to scan directory: {directory}", ex);
            }
        }
    }
}
