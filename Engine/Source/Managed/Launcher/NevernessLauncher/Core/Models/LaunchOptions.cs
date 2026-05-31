using NevernessLauncher.Core.Enums;
using System.Collections.Generic;

namespace NevernessLauncher.Core.Models
{
    /// <summary>
    /// 启动选项模型
    /// </summary>
    public class LaunchOptions
    {
        /// <summary>启动模式</summary>
        public LaunchMode Mode { get; set; } = LaunchMode.Normal;

        /// <summary>是否启用调试</summary>
        public bool Debug { get; set; }

        /// <summary>窗口模式</summary>
        public bool Windowed { get; set; } = true;

        /// <summary>窗口宽度</summary>
        public int? Width { get; set; }

        /// <summary>窗口高度</summary>
        public int? Height { get; set; }

        /// <summary>额外命令行参数</summary>
        public string? AdditionalArgs { get; set; }

        /// <summary>额外环境变量</summary>
        public Dictionary<string, string>? EnvironmentVars { get; set; }

        /// <summary>是否重定向输出</summary>
        public bool RedirectOutput { get; set; }

        /// <summary>是否以管理员运行</summary>
        public bool RunAsAdmin { get; set; }

        /// <summary>初始场景路径</summary>
        public string? InitialScene { get; set; }
    }
}
