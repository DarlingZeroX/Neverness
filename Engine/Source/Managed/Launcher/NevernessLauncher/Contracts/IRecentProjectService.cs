using NevernessLauncher.Core.Models;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace NevernessLauncher.Contracts
{
    /// <summary>
    /// 最近项目服务接口
    /// </summary>
    public interface IRecentProjectService
    {
        /// <summary>获取最近项目列表</summary>
        IReadOnlyList<RecentProject> GetRecentProjects();

        /// <summary>记录项目访问</summary>
        Task RecordAccess(string projectPath);

        /// <summary>从最近项目列表移除</summary>
        Task RemoveRecent(string projectPath);

        /// <summary>清除所有最近项目</summary>
        Task ClearAll();

        /// <summary>置顶项目</summary>
        Task PinProject(string projectPath);

        /// <summary>取消置顶项目</summary>
        Task UnpinProject(string projectPath);

        /// <summary>获取置顶项目列表</summary>
        IReadOnlyList<RecentProject> GetPinnedProjects();

        /// <summary>检查项目是否有效</summary>
        bool IsValid(string projectPath);

        /// <summary>最近项目列表变更事件</summary>
        event EventHandler? RecentProjectsChanged;
    }
}
