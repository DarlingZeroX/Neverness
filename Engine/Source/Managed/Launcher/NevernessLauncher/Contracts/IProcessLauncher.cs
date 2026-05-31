using NevernessLauncher.Core.Models;

namespace NevernessLauncher.Contracts
{
    /// <summary>
    /// 进程启动接口
    /// </summary>
    public interface IProcessLauncher
    {
        /// <summary>启动进程</summary>
        ProcessHandle Launch(string executablePath, string arguments, LaunchOptions? options = null);

        /// <summary>启动进程（指定工作目录）</summary>
        ProcessHandle Launch(string executablePath, string arguments, string workingDirectory);

        /// <summary>检查进程是否正在运行</summary>
        bool IsProcessRunning(int processId);

        /// <summary>终止进程</summary>
        void KillProcess(int processId);
    }
}
