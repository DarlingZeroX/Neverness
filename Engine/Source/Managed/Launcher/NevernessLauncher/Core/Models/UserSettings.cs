using Avalonia;
using System.Collections.Generic;

namespace NevernessLauncher.Core.Models
{
    /// <summary>
    /// 用户偏好设置模型
    /// </summary>
    public class UserSettings
    {
        /// <summary>主题: Light/Dark/System</summary>
        public string Theme { get; set; } = "Dark";

        /// <summary>界面语言</summary>
        public string Language { get; set; } = "zh-CN";

        /// <summary>窗口状态</summary>
        public string WindowState { get; set; } = "Normal";

        /// <summary>窗口位置 X</summary>
        public int? WindowPositionX { get; set; }

        /// <summary>窗口位置 Y</summary>
        public int? WindowPositionY { get; set; }

        /// <summary>窗口宽度</summary>
        public int? WindowWidth { get; set; }

        /// <summary>窗口高度</summary>
        public int? WindowHeight { get; set; }

        /// <summary>自定义引擎扫描路径</summary>
        public List<string> EnginePaths { get; set; } = new();

        /// <summary>项目扫描目录</summary>
        public List<string> ProjectDirectories { get; set; } = new();

        /// <summary>默认引擎版本</summary>
        public string? DefaultEngineVersion { get; set; }

        /// <summary>启动时自动扫描</summary>
        public bool AutoScanOnStartup { get; set; } = true;

        /// <summary>最大最近项目数</summary>
        public int MaxRecentProjects { get; set; } = 20;

        /// <summary>上次活跃页面</summary>
        public string LastActivePage { get; set; } = "Home";

        /// <summary>上次打开的项目路径</summary>
        public string? LastOpenedProject { get; set; }
    }
}
