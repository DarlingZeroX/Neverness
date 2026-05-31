using NevernessLauncher.Core.Enums;

namespace NevernessLauncher.Core.Constants
{
    /// <summary>
    /// 默认值常量定义
    /// </summary>
    public static class DefaultConstants
    {
        /// <summary>默认主题</summary>
        public const string DefaultTheme = "System";

        /// <summary>默认语言</summary>
        public const string DefaultLanguage = "zh-CN";

        /// <summary>默认启动时自动扫描</summary>
        public const bool DefaultAutoScanOnStartup = true;

        /// <summary>默认最大最近项目数</summary>
        public const int DefaultMaxRecentProjects = 20;

        /// <summary>默认窗口宽度</summary>
        public const int DefaultWindowWidth = 1200;

        /// <summary>默认窗口高度</summary>
        public const int DefaultWindowHeight = 800;

        /// <summary>默认启动模式</summary>
        public const LaunchMode DefaultLaunchMode = LaunchMode.Normal;

        /// <summary>默认引擎渠道</summary>
        public const EngineChannel DefaultEngineChannel = EngineChannel.Stable;

        /// <summary>无效项目自动移除天数</summary>
        public const int InvalidProjectAutoRemoveDays = 30;

        /// <summary>日志最大保留天数</summary>
        public const int LogMaxRetainDays = 30;

        /// <summary>日志最大文件大小 (MB)</summary>
        public const int LogMaxFileSizeMB = 10;

        /// <summary>日志目录最大总大小 (MB)</summary>
        public const int LogMaxTotalSizeMB = 100;
    }
}
