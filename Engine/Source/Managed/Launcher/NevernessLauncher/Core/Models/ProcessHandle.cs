using NevernessLauncher.Core.Enums;
using System;

namespace NevernessLauncher.Core.Models
{
    /// <summary>
    /// 进程句柄模型
    /// </summary>
    public class ProcessHandle
    {
        /// <summary>进程 ID</summary>
        public int ProcessId { get; set; }

        /// <summary>关联的项目 ID</summary>
        public string ProjectId { get; set; } = string.Empty;

        /// <summary>进程类型</summary>
        public ProcessType ProcessType { get; set; }

        /// <summary>启动时间</summary>
        public DateTimeOffset StartTime { get; set; } = DateTimeOffset.Now;

        /// <summary>是否仍在运行</summary>
        public bool IsRunning { get; set; } = true;

        /// <summary>退出码（退出后有效）</summary>
        public int? ExitCode { get; set; }
    }
}
