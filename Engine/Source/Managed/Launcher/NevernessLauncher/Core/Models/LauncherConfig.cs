using System;

namespace NevernessLauncher.Core.Models
{
    /// <summary>
    /// Launcher 应用级配置（只读）
    /// </summary>
    public class LauncherConfig
    {
        /// <summary>Launcher 安装目录</summary>
        public string InstallPath { get; set; } = string.Empty;

        /// <summary>Launcher 版本号</summary>
        public Version Version { get; set; } = new Version(1, 0, 0);

        /// <summary>数据存储目录</summary>
        public string DataDirectory { get; set; } = string.Empty;
    }
}
