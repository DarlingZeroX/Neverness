using NevernessLauncher.Core.Models;
using System.Threading.Tasks;

namespace NevernessLauncher.Contracts
{
    /// <summary>
    /// 配置服务接口
    /// </summary>
    public interface IConfigurationService
    {
        /// <summary>加载 Launcher 应用配置</summary>
        LauncherConfig LoadLauncherConfig();

        /// <summary>加载用户设置</summary>
        UserSettings LoadUserSettings();

        /// <summary>保存用户设置</summary>
        Task SaveUserSettings(UserSettings settings);

        /// <summary>加载最近项目缓存</summary>
        RecentProjectsCache LoadRecentProjects();

        /// <summary>保存最近项目缓存</summary>
        Task SaveRecentProjects(RecentProjectsCache cache);

        /// <summary>获取数据存储目录</summary>
        string GetDataDirectory();

        /// <summary>获取用户设置文件路径</summary>
        string GetUserSettingsPath();

        /// <summary>获取最近项目文件路径</summary>
        string GetRecentProjectsPath();
    }
}
