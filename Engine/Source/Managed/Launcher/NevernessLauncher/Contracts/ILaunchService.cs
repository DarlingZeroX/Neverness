using NevernessLauncher.Core.Models;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace NevernessLauncher.Contracts
{
    /// <summary>
    /// 启动服务接口
    /// </summary>
    public interface ILaunchService
    {
        /// <summary>启动 Editor</summary>
        Task<ProcessHandle> LaunchEditor(ProjectInfo project, LaunchOptions? options = null);

        /// <summary>启动 Game/Runtime</summary>
        Task<ProcessHandle> LaunchGame(ProjectInfo project, LaunchOptions? options = null);

        /// <summary>检查 Editor 是否正在运行</summary>
        bool IsEditorRunning(string projectId);

        /// <summary>检查 Game 是否正在运行</summary>
        bool IsGameRunning(string projectId);

        /// <summary>获取所有运行中的 Editor 进程</summary>
        IReadOnlyList<ProcessHandle> GetRunningEditors();

        /// <summary>获取所有运行中的 Game 进程</summary>
        IReadOnlyList<ProcessHandle> GetRunningGames();

        /// <summary>等待进程退出</summary>
        Task<int> WaitForExit(ProcessHandle handle);

        /// <summary>终止进程</summary>
        void KillProcess(ProcessHandle handle);

        /// <summary>启动前检查</summary>
        PreLaunchCheckResult PreLaunchCheck(ProjectInfo project);

        /// <summary>进程启动事件</summary>
        event EventHandler<ProcessHandle>? ProcessStarted;

        /// <summary>进程退出事件</summary>
        event EventHandler<ProcessHandle>? ProcessExited;

        /// <summary>进程启动失败事件</summary>
        event EventHandler<string>? ProcessFailed;
    }
}
