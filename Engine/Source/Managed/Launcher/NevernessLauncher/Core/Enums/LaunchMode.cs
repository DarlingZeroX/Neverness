namespace NevernessLauncher.Core.Enums
{
    /// <summary>
    /// 启动模式枚举
    /// </summary>
    public enum LaunchMode
    {
        /// <summary>正常启动</summary>
        Normal,
        /// <summary>调试模式</summary>
        Debug,
        /// <summary>开发模式</summary>
        Development,
        /// <summary>性能分析模式</summary>
        Profile,
        /// <summary>无头模式（仅 Runtime）</summary>
        Headless
    }
}
