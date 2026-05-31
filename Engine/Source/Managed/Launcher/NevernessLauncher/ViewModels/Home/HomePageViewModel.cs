using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using NevernessLauncher.Contracts;
using NevernessLauncher.Core.Models;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading.Tasks;

namespace NevernessLauncher.ViewModels.Home
{
    /// <summary>
    /// 首页 ViewModel（项目列表）
    /// </summary>
    public partial class HomePageViewModel : PageViewModelBase
    {
        private readonly ILogService _logService;
        private readonly IProjectService _projectService;
        private readonly ILaunchService _launchService;
        private readonly IRecentProjectService _recentProjectService;

        /// <summary>项目列表</summary>
        public ObservableCollection<ProjectCardViewModel> Projects { get; } = new();

        /// <summary>置顶项目</summary>
        public ObservableCollection<ProjectCardViewModel> PinnedProjects { get; } = new();

        /// <summary>最近项目</summary>
        public ObservableCollection<ProjectCardViewModel> RecentProjects { get; } = new();

        /// <summary>选中项目</summary>
        [ObservableProperty]
        private ProjectCardViewModel? _selectedProject;

        /// <summary>是否正在扫描</summary>
        [ObservableProperty]
        private bool _isScanning;

        /// <summary>搜索文本</summary>
        [ObservableProperty]
        private string _searchText = string.Empty;

        public HomePageViewModel(
            ILogService logService,
            IProjectService projectService,
            ILaunchService launchService,
            IRecentProjectService recentProjectService)
        {
            _logService = logService;
            _projectService = projectService;
            _launchService = launchService;
            _recentProjectService = recentProjectService;

            Title = "Projects";
            IconKey = "project";
        }

        /// <summary>页面进入时调用</summary>
        public override async Task OnNavigatedTo(object? parameter = null)
        {
            await ScanProjects();
        }

        /// <summary>扫描项目</summary>
        [RelayCommand]
        private async Task ScanProjects()
        {
            if (IsScanning)
                return;

            IsScanning = true;
            _logService.LogInfo("Scanning projects...");

            try
            {
                var projects = await _projectService.ScanProjects();

                Projects.Clear();
                PinnedProjects.Clear();
                RecentProjects.Clear();

                foreach (var project in projects)
                {
                    var card = new ProjectCardViewModel(project, _launchService, _recentProjectService);
                    Projects.Add(card);

                    if (project.LastOpenedTime.HasValue)
                    {
                        RecentProjects.Add(card);
                    }
                }

                // 从最近项目获取置顶项目
                var pinnedProjects = _recentProjectService.GetPinnedProjects();
                foreach (var pinned in pinnedProjects)
                {
                    var card = Projects.FirstOrDefault(p => p.ProjectInfo.ProjectPath == pinned.ProjectPath);
                    if (card != null)
                    {
                        PinnedProjects.Add(card);
                    }
                }

                _logService.LogInfo($"Scan completed: {Projects.Count} projects found");
            }
            catch (System.Exception ex)
            {
                _logService.LogError("Failed to scan projects", ex);
            }
            finally
            {
                IsScanning = false;
            }
        }

        /// <summary>打开项目文件</summary>
        [RelayCommand]
        private async Task OpenProject()
        {
            var project = await _projectService.OpenProjectFile();
            if (project != null)
            {
                await _recentProjectService.RecordAccess(project.ProjectFilePath);
                await ScanProjects();
            }
        }

        /// <summary>创建新项目</summary>
        [RelayCommand]
        private async Task CreateProject()
        {
            // TODO: 实现创建项目向导
            _logService.LogInfo("Create project dialog requested");
        }

        /// <summary>启动 Editor</summary>
        [RelayCommand]
        private async Task LaunchEditor(ProjectCardViewModel? projectCard)
        {
            if (projectCard == null)
                return;

            try
            {
                await projectCard.LaunchEditorCommand.ExecuteAsync(null);
            }
            catch (System.Exception ex)
            {
                _logService.LogError("Failed to launch Editor", ex);
            }
        }

        /// <summary>启动 Game</summary>
        [RelayCommand]
        private async Task LaunchGame(ProjectCardViewModel? projectCard)
        {
            if (projectCard == null)
                return;

            try
            {
                await projectCard.LaunchGameCommand.ExecuteAsync(null);
            }
            catch (System.Exception ex)
            {
                _logService.LogError("Failed to launch Game", ex);
            }
        }
    }
}
