using NevernessLauncher.Core.Models;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace NevernessLauncher.Contracts
{
    /// <summary>
    /// 项目服务接口
    /// </summary>
    public interface IProjectService
    {
        /// <summary>扫描项目</summary>
        Task<IReadOnlyList<ProjectInfo>> ScanProjects();

        /// <summary>获取所有项目列表</summary>
        IReadOnlyList<ProjectInfo> GetProjects();

        /// <summary>从 .nnproject 文件加载项目信息</summary>
        ProjectInfo? LoadProject(string projectFilePath);

        /// <summary>验证项目是否有效</summary>
        bool ValidateProject(ProjectInfo project);

        /// <summary>打开项目文件（通过文件对话框）</summary>
        Task<ProjectInfo?> OpenProjectFile();

        /// <summary>添加项目扫描目录</summary>
        Task AddProjectDirectory(string directory);

        /// <summary>移除项目扫描目录</summary>
        Task RemoveProjectDirectory(string directory);

        /// <summary>获取项目扫描目录列表</summary>
        IReadOnlyList<string> GetProjectDirectories();

        /// <summary>项目列表变更事件</summary>
        event EventHandler? ProjectsChanged;
    }
}
