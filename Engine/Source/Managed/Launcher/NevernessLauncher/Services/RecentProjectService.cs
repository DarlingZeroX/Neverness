using NevernessLauncher.Contracts;
using NevernessLauncher.Core.Constants;
using NevernessLauncher.Core.Models;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

namespace NevernessLauncher.Services
{
    /// <summary>
    /// 最近项目服务实现
    /// </summary>
    public class RecentProjectService : IRecentProjectService
    {
        private readonly ILogService _logService;
        private readonly IConfigurationService _configService;
        private readonly IFileSystem _fileSystem;
        private RecentProjectsCache _cache;

        public event EventHandler? RecentProjectsChanged;

        public RecentProjectService(
            ILogService logService,
            IConfigurationService configService,
            IFileSystem fileSystem)
        {
            _logService = logService;
            _configService = configService;
            _fileSystem = fileSystem;
            _cache = _configService.LoadRecentProjects();
        }

        /// <summary>获取最近项目列表</summary>
        public IReadOnlyList<RecentProject> GetRecentProjects()
        {
            return _cache.Projects.AsReadOnly();
        }

        /// <summary>记录项目访问</summary>
        public async Task RecordAccess(string projectPath)
        {
            var existing = _cache.Projects.FirstOrDefault(p => p.ProjectPath == projectPath);
            if (existing != null)
            {
                existing.LastAccessTime = DateTimeOffset.Now;
                existing.AccessCount++;
            }
            else
            {
                var projectName = Path.GetFileNameWithoutExtension(projectPath);
                _cache.Projects.Add(new RecentProject
                {
                    ProjectPath = projectPath,
                    ProjectName = projectName,
                    LastAccessTime = DateTimeOffset.Now,
                    AccessCount = 1,
                    IsValid = true
                });
            }

            // 限制最大数量
            var settings = _configService.LoadUserSettings();
            while (_cache.Projects.Count > settings.MaxRecentProjects)
            {
                var oldest = _cache.Projects
                    .Where(p => !p.IsPinned)
                    .OrderBy(p => p.LastAccessTime)
                    .FirstOrDefault();

                if (oldest != null)
                {
                    _cache.Projects.Remove(oldest);
                }
                else
                {
                    break;
                }
            }

            await SaveCache();
            _logService.LogInfo($"Recorded project access: {projectPath}");
            RecentProjectsChanged?.Invoke(this, EventArgs.Empty);
        }

        /// <summary>从最近项目列表移除</summary>
        public async Task RemoveRecent(string projectPath)
        {
            var project = _cache.Projects.FirstOrDefault(p => p.ProjectPath == projectPath);
            if (project != null)
            {
                _cache.Projects.Remove(project);
                await SaveCache();
                _logService.LogInfo($"Removed from recent projects: {projectPath}");
                RecentProjectsChanged?.Invoke(this, EventArgs.Empty);
            }
        }

        /// <summary>清除所有最近项目</summary>
        public async Task ClearAll()
        {
            _cache.Projects.Clear();
            await SaveCache();
            _logService.LogInfo("Cleared all recent projects");
            RecentProjectsChanged?.Invoke(this, EventArgs.Empty);
        }

        /// <summary>置顶项目</summary>
        public async Task PinProject(string projectPath)
        {
            var project = _cache.Projects.FirstOrDefault(p => p.ProjectPath == projectPath);
            if (project != null)
            {
                project.IsPinned = true;
                await SaveCache();
                _logService.LogInfo($"Pinned project: {projectPath}");
                RecentProjectsChanged?.Invoke(this, EventArgs.Empty);
            }
        }

        /// <summary>取消置顶项目</summary>
        public async Task UnpinProject(string projectPath)
        {
            var project = _cache.Projects.FirstOrDefault(p => p.ProjectPath == projectPath);
            if (project != null)
            {
                project.IsPinned = false;
                await SaveCache();
                _logService.LogInfo($"Unpinned project: {projectPath}");
                RecentProjectsChanged?.Invoke(this, EventArgs.Empty);
            }
        }

        /// <summary>获取置顶项目列表</summary>
        public IReadOnlyList<RecentProject> GetPinnedProjects()
        {
            return _cache.Projects
                .Where(p => p.IsPinned)
                .OrderByDescending(p => p.LastAccessTime)
                .ToList()
                .AsReadOnly();
        }

        /// <summary>检查项目是否有效</summary>
        public bool IsValid(string projectPath)
        {
            return _fileSystem.FileExists(projectPath);
        }

        private async Task SaveCache()
        {
            await _configService.SaveRecentProjects(_cache);
        }
    }
}
