using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using NevernessLauncher.Contracts;
using NevernessLauncher.Core.Enums;
using NevernessLauncher.Core.Models;
using System;
using System.Threading.Tasks;

namespace NevernessLauncher.ViewModels.Home
{
    /// <summary>
    /// 项目卡片 ViewModel
    /// </summary>
    public partial class ProjectCardViewModel : ViewModelBase
    {
        private readonly ILaunchService _launchService;
        private readonly IRecentProjectService _recentProjectService;
        private readonly ILocalizationService _localizationService;

        /// <summary>项目领域模型</summary>
        public ProjectInfo ProjectInfo { get; }

        /// <summary>项目名称</summary>
        public string DisplayName => ProjectInfo.Name;

        /// <summary>项目路径</summary>
        public string ProjectPath => ProjectInfo.ProjectPath;

        /// <summary>引擎版本</summary>
        public string EngineVersion => ProjectInfo.EngineVersion;

        /// <summary>最后打开时间</summary>
        public DateTimeOffset? LastOpened => ProjectInfo.LastOpenedTime;

        /// <summary>项目状态</summary>
        [ObservableProperty]
        private ProjectStatus _status;

        /// <summary>是否置顶</summary>
        [ObservableProperty]
        private bool _isPinned;

        /// <summary>是否正在运行</summary>
        [ObservableProperty]
        private bool _isRunning;

        /// <summary>状态颜色</summary>
        [ObservableProperty]
        private string _statusColor = "Green";

        /// <summary>状态文本</summary>
        [ObservableProperty]
        private string _statusText = "Valid";

        /// <summary>编辑器按钮文本</summary>
        [ObservableProperty]
        private string _editorText = "Editor";

        /// <summary>运行按钮文本</summary>
        [ObservableProperty]
        private string _playText = "Play";

        public ProjectCardViewModel(
            ProjectInfo projectInfo,
            ILaunchService launchService,
            IRecentProjectService recentProjectService,
            ILocalizationService localizationService)
        {
            ProjectInfo = projectInfo;
            _launchService = launchService;
            _recentProjectService = recentProjectService;
            _localizationService = localizationService;

            Status = projectInfo.Status;
            UpdateStatusColor();
            UpdateLocalizedTexts();
        }

        /// <summary>启动 Editor</summary>
        [RelayCommand]
        private async Task LaunchEditor()
        {
            try
            {
                IsRunning = true;
                var handle = await _launchService.LaunchEditor(ProjectInfo);
                await _recentProjectService.RecordAccess(ProjectInfo.ProjectFilePath);

                // 异步等待进程退出
                _ = Task.Run(async () =>
                {
                    await _launchService.WaitForExit(handle);
                    IsRunning = false;
                });
            }
            catch (Exception ex)
            {
                IsRunning = false;
                throw;
            }
        }

        /// <summary>启动 Game</summary>
        [RelayCommand]
        private async Task LaunchGame()
        {
            try
            {
                IsRunning = true;
                var handle = await _launchService.LaunchGame(ProjectInfo);
                await _recentProjectService.RecordAccess(ProjectInfo.ProjectFilePath);

                // 异步等待进程退出
                _ = Task.Run(async () =>
                {
                    await _launchService.WaitForExit(handle);
                    IsRunning = false;
                });
            }
            catch (Exception ex)
            {
                IsRunning = false;
                throw;
            }
        }

        /// <summary>置顶/取消置顶</summary>
        [RelayCommand]
        private async Task TogglePin()
        {
            if (IsPinned)
            {
                await _recentProjectService.UnpinProject(ProjectInfo.ProjectFilePath);
                IsPinned = false;
            }
            else
            {
                await _recentProjectService.PinProject(ProjectInfo.ProjectFilePath);
                IsPinned = true;
            }
        }

        /// <summary>打开项目文件夹</summary>
        [RelayCommand]
        private void OpenFolder()
        {
            try
            {
                System.Diagnostics.Process.Start("explorer.exe", ProjectInfo.ProjectPath);
            }
            catch
            {
                // 静默处理
            }
        }

        private void UpdateStatusColor()
        {
            StatusColor = Status switch
            {
                ProjectStatus.Valid => "#4CAF50",
                ProjectStatus.EngineMissing => "#FF9800",
                ProjectStatus.InvalidPath => "#F44336",
                ProjectStatus.Corrupted => "#F44336",
                ProjectStatus.VersionMismatch => "#FF9800",
                _ => "#999999"
            };

            StatusText = Status switch
            {
                ProjectStatus.Valid => _localizationService.GetString("ProjectStatus.Valid"),
                ProjectStatus.EngineMissing => _localizationService.GetString("ProjectStatus.EngineMissing"),
                ProjectStatus.InvalidPath => _localizationService.GetString("ProjectStatus.InvalidPath"),
                ProjectStatus.Corrupted => _localizationService.GetString("ProjectStatus.Corrupted"),
                ProjectStatus.VersionMismatch => _localizationService.GetString("ProjectStatus.VersionMismatch"),
                _ => Status.ToString()
            };
        }

        private void UpdateLocalizedTexts()
        {
            EditorText = _localizationService.GetString("Home.Editor");
            PlayText = _localizationService.GetString("Home.Play");
            UpdateStatusColor();
        }
    }
}
