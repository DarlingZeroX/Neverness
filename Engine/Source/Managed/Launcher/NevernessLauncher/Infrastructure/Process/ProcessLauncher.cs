using NevernessLauncher.Contracts;
using NevernessLauncher.Core.Models;
using System;
using System.Diagnostics;

namespace NevernessLauncher.Infrastructure.Process
{
    /// <summary>
    /// 进程启动实现
    /// </summary>
    public class ProcessLauncher : IProcessLauncher
    {
        private readonly ILogService _logService;

        public ProcessLauncher(ILogService logService)
        {
            _logService = logService;
        }

        /// <summary>启动进程</summary>
        public ProcessHandle Launch(string executablePath, string arguments, LaunchOptions? options = null)
        {
            try
            {
                var startInfo = new ProcessStartInfo
                {
                    FileName = executablePath,
                    Arguments = arguments,
                    UseShellExecute = false,
                    CreateNoWindow = false
                };

                // 设置工作目录
                if (options != null)
                {
                    startInfo.RedirectStandardOutput = options.RedirectOutput;
                    startInfo.RedirectStandardError = options.RedirectOutput;

                    // 添加环境变量
                    if (options.EnvironmentVars != null)
                    {
                        foreach (var envVar in options.EnvironmentVars)
                        {
                            startInfo.EnvironmentVariables[envVar.Key] = envVar.Value;
                        }
                    }
                }

                var process = System.Diagnostics.Process.Start(startInfo);
                if (process == null)
                {
                    throw new InvalidOperationException($"Failed to start process: {executablePath}");
                }

                _logService.LogInfo($"Process started: {executablePath} (PID: {process.Id})");

                return new ProcessHandle
                {
                    ProcessId = process.Id,
                    ProcessType = Core.Enums.ProcessType.Runtime,
                    StartTime = DateTimeOffset.Now,
                    IsRunning = true
                };
            }
            catch (Exception ex)
            {
                _logService.LogError($"Failed to start process: {executablePath}", ex);
                throw;
            }
        }

        /// <summary>启动进程（指定工作目录）</summary>
        public ProcessHandle Launch(string executablePath, string arguments, string workingDirectory)
        {
            try
            {
                var startInfo = new ProcessStartInfo
                {
                    FileName = executablePath,
                    Arguments = arguments,
                    WorkingDirectory = workingDirectory,
                    UseShellExecute = false,
                    CreateNoWindow = false
                };

                var process = System.Diagnostics.Process.Start(startInfo);
                if (process == null)
                {
                    throw new InvalidOperationException($"Failed to start process: {executablePath}");
                }

                _logService.LogInfo($"Process started: {executablePath} (PID: {process.Id})");

                return new ProcessHandle
                {
                    ProcessId = process.Id,
                    ProcessType = Core.Enums.ProcessType.Runtime,
                    StartTime = DateTimeOffset.Now,
                    IsRunning = true
                };
            }
            catch (Exception ex)
            {
                _logService.LogError($"Failed to start process: {executablePath}", ex);
                throw;
            }
        }

        /// <summary>检查进程是否正在运行</summary>
        public bool IsProcessRunning(int processId)
        {
            try
            {
                var process = System.Diagnostics.Process.GetProcessById(processId);
                return !process.HasExited;
            }
            catch
            {
                return false;
            }
        }

        /// <summary>终止进程</summary>
        public void KillProcess(int processId)
        {
            try
            {
                var process = System.Diagnostics.Process.GetProcessById(processId);
                if (!process.HasExited)
                {
                    process.Kill();
                    _logService.LogInfo($"Process killed: PID {processId}");
                }
            }
            catch (Exception ex)
            {
                _logService.LogError($"Failed to kill process: PID {processId}", ex);
            }
        }
    }
}
