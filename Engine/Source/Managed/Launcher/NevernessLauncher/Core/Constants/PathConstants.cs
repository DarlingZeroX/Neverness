namespace NevernessLauncher.Core.Constants
{
    /// <summary>
    /// 路径常量定义
    /// </summary>
    public static class PathConstants
    {
        /// <summary>Neverness 注册表路径 (HKLM)</summary>
        public const string RegistryPathHKLM = @"SOFTWARE\Neverness\Engine\Installations";

        /// <summary>Neverness 注册表路径 (HKCU)</summary>
        public const string RegistryPathHKCU = @"SOFTWARE\Neverness\Engine\Installations";

        /// <summary>环境变量名 - 引擎根目录</summary>
        public const string EnvEngineRoot = "NEVERNESS_ENGINE_ROOT";

        /// <summary>环境变量名 - 调试模式</summary>
        public const string EnvDebug = "NEVERNESS_DEBUG";

        /// <summary>环境变量名 - 日志级别</summary>
        public const string EnvLogLevel = "NEVERNESS_LOG_LEVEL";

        /// <summary>固定引擎扫描路径 - Windows</summary>
        public static readonly string[] FixedEnginePaths = new[]
        {
            @"C:\Neverness\Engine",
            @"D:\Neverness\Engine",
            @"E:\Neverness\Engine"
        };

        /// <summary>Editor 可执行文件相对路径</summary>
        public const string EditorRelativePath = @"Bin\Editor\NevernessEditor.exe";

        /// <summary>Runtime 可执行文件相对路径</summary>
        public const string RuntimeRelativePath = @"Bin\Runtime\NevernessRuntime.exe";

        /// <summary>配置目录名</summary>
        public const string ConfigDirectoryName = "Neverness";

        /// <summary>Launcher 数据目录名</summary>
        public const string LauncherDataDirectoryName = "Launcher";

        /// <summary>设置文件名</summary>
        public const string SettingsFileName = "settings.json";

        /// <summary>最近项目文件名</summary>
        public const string RecentProjectsFileName = "recent_projects.json";

        /// <summary>日志目录名</summary>
        public const string LogsDirectoryName = "Logs";
    }
}
