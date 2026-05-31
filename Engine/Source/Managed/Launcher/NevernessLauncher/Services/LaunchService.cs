using NevernessLauncher.Contracts;
using NevernessLauncher.Core.Constants;
using NevernessLauncher.Core.Enums;
using NevernessLauncher.Core.Models;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Threading.Tasks;

namespace NevernessLauncher.Services
{
    /// <summary>
    /// 启动服务实现
    /// </summary>
    public class LaunchService : ILaunchService
    {
        private readonly ILogService _logService;
        private readonly IEngineService _engineService;
        private readonly IProcessLauncher _processLauncher;
        private readonly List<ProcessHandle> _runningProcesses = new();

        public event EventHandler<ProcessHandle>? ProcessStarted;
        public event EventHandler<ProcessHandle>? ProcessExited;
        public event EventHandler<string>? ProcessFailed;

        public LaunchService(
            ILogService logService,
            IEngineService engineService,
            IProcessLauncher processLauncher)
        {
            _logService = logService;
            _engineService = engineService;
            _processLauncher = processLauncher;
        }

        /// <summary>启动 Editor</summary>
        public async Task<ProcessHandle> LaunchEditor(ProjectInfo project, LaunchOptions? options = null)
        {
            _logService.LogInfo($"Launching Editor for project: {project.Name}");

            // 查找引擎
            var engine = FindEngineForProject(project);
            if (engine == null)
            {
                var errorMsg = $"Engine not found for project: {project.Name} (requires version {project.EngineVersion})";
                _logService.LogError(errorMsg);
                ProcessFailed?.Invoke(this, errorMsg);
                throw new InvalidOperationException(errorMsg);
            }

            // 构建启动参数
            var arguments = BuildEditorArguments(project, engine, options);

            try
            {
                var handle = _processLauncher.Launch(engine.EditorPath, arguments, options);
                handle.ProjectId = project.Id.ToString();
                handle.ProcessType = ProcessType.Editor;

                _runningProcesses.Add(handle);
                ProcessStarted?.Invoke(this, handle);

                _logService.LogInfo($"Editor launched: PID={handle.ProcessId}");
                return handle;
            }
            catch (Exception ex)
            {
                _logService.LogError("Failed to launch Editor", ex);
                ProcessFailed?.Invoke(this, ex.Message);
                throw;
            }
        }

        /// <summary>启动 Game/Runtime</summary>
        public async Task<ProcessHandle> LaunchGame(ProjectInfo project, LaunchOptions? options = null)
        {
            _logService.LogInfo($"Launching Game for project: {project.Name}");

            // 查找引擎
            var engine = FindEngineForProject(project);
            if (engine == null)
            {
                var errorMsg = $"Engine not found for project: {project.Name} (requires version {project.EngineVersion})";
                _logService.LogError(errorMsg);
                ProcessFailed?.Invoke(this, errorMsg);
                throw new InvalidOperationException(errorMsg);
            }

            // 构建启动参数
            var arguments = BuildGameArguments(project, engine, options);

            try
            {
                var handle = _processLauncher.Launch(engine.RuntimePath, arguments, options);
                handle.ProjectId = project.Id.ToString();
                handle.ProcessType = ProcessType.Runtime;

                _runningProcesses.Add(handle);
                ProcessStarted?.Invoke(this, handle);

                _logService.LogInfo($"Game launched: PID={handle.ProcessId}");
                return handle;
            }
            catch (Exception ex)
            {
                _logService.LogError("Failed to launch Game", ex);
                ProcessFailed?.Invoke(this, ex.Message);
                throw;
            }
        }

        /// <summary>检查 Editor 是否正在运行</summary>
        public bool IsEditorRunning(string projectId)
        {
            return _runningProcesses.Any(p => p.ProjectId == projectId && p.ProcessType == ProcessType.Editor && p.IsRunning);
        }

        /// <summary>检查 Game 是否正在运行</summary>
        public bool IsGameRunning(string projectId)
        {
            return _runningProcesses.Any(p => p.ProjectId == projectId && p.ProcessType == ProcessType.Runtime && p.IsRunning);
        }

        /// <summary>获取所有运行中的 Editor 进程</summary>
        public IReadOnlyList<ProcessHandle> GetRunningEditors()
        {
            return _runningProcesses
                .Where(p => p.ProcessType == ProcessType.Editor && p.IsRunning)
                .ToList()
                .AsReadOnly();
        }

        /// <summary>获取所有运行中的 Game 进程</summary>
        public IReadOnlyList<ProcessHandle> GetRunningGames()
        {
            return _runningProcesses
                .Where(p => p.ProcessType == ProcessType.Runtime && p.IsRunning)
                .ToList()
                .AsReadOnly();
        }

        /// <summary>等待进程退出</summary>
        public async Task<int> WaitForExit(ProcessHandle handle)
        {
            return await Task.Run(() =>
            {
                while (_processLauncher.IsProcessRunning(handle.ProcessId))
                {
                    System.Threading.Thread.Sleep(100);
                }
                handle.IsRunning = false;
                ProcessExited?.Invoke(this, handle);
                return handle.ExitCode ?? 0;
            });
        }

        /// <summary>终止进程</summary>
        public void KillProcess(ProcessHandle handle)
        {
            _processLauncher.KillProcess(handle.ProcessId);
            handle.IsRunning = false;
            _logService.LogInfo($"Process killed: PID={handle.ProcessId}");
            ProcessExited?.Invoke(this, handle);
        }

        /// <summary>启动前检查</summary>
        public PreLaunchCheckResult PreLaunchCheck(ProjectInfo project)
        {
            var result = new PreLaunchCheckResult();

            // 检查项目路径
            if (string.IsNullOrEmpty(project.ProjectPath) || !Directory.Exists(project.ProjectPath))
            {
                result.CanLaunch = false;
                result.Issues.Add("Project path is invalid or does not exist");
            }

            // 检查引擎版本
            var engine = FindEngineForProject(project);
            if (engine == null)
            {
                result.CanLaunch = false;
                result.Issues.Add($"Engine version {project.EngineVersion} not found");
            }
            else if (!engine.IsValid)
            {
                result.CanLaunch = false;
                result.Issues.Add("Engine installation is invalid");
            }

            return result;
        }

        private EngineVersion? FindEngineForProject(ProjectInfo project)
        {
            if (Version.TryParse(project.EngineVersion, out var version))
            {
                return _engineService.FindEngine(version)
                    ?? _engineService.FindCompatibleEngine(version);
            }

            return _engineService.GetDefaultEngine();
        }

        private string BuildEditorArguments(ProjectInfo project, EngineVersion engine, LaunchOptions? options)
        {
            var args = new List<string>
            {
                $"--project \"{project.ProjectPath}\"",
                $"--engine \"{engine.InstallPath}\"",
                "--mode editor"
            };

            if (options?.Debug == true)
            {
                args.Add("--debug");
            }

            if (!string.IsNullOrEmpty(options?.InitialScene))
            {
                args.Add($"--scene \"{options.InitialScene}\"");
            }

            if (!string.IsNullOrEmpty(options?.AdditionalArgs))
            {
                args.Add(options.AdditionalArgs);
            }

            return string.Join(" ", args);
        }

        private string BuildGameArguments(ProjectInfo project, EngineVersion engine, LaunchOptions? options)
        {
            var args = new List<string>
            {
                $"--project \"{project.ProjectPath}\"",
                "--mode game"
            };

            if (options?.Windowed == true)
            {
                args.Add("--windowed");
            }
            else
            {
                args.Add("--fullscreen");
            }

            if (options?.Width.HasValue == true)
            {
                args.Add($"--width {options.Width}");
            }

            if (options?.Height.HasValue == true)
            {
                args.Add($"--height {options.Height}");
            }

            if (options?.Debug == true)
            {
                args.Add("--debug");
            }

            if (!string.IsNullOrEmpty(options?.AdditionalArgs))
            {
                args.Add(options.AdditionalArgs);
            }

            return string.Join(" ", args);
        }
    }
}
