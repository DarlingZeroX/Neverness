using System;

namespace NevernessLauncher.Contracts
{
    /// <summary>
    /// 日志服务接口
    /// </summary>
    public interface ILogService
    {
        /// <summary>记录 Trace 级别日志</summary>
        void LogTrace(string message);

        /// <summary>记录 Debug 级别日志</summary>
        void LogDebug(string message);

        /// <summary>记录 Info 级别日志</summary>
        void LogInfo(string message);

        /// <summary>记录 Warning 级别日志</summary>
        void LogWarning(string message);

        /// <summary>记录 Error 级别日志</summary>
        void LogError(string message, Exception? exception = null);

        /// <summary>记录 Critical 级别日志</summary>
        void LogCritical(string message, Exception? exception = null);

        /// <summary>获取日志目录路径</summary>
        string GetLogDirectory();

        /// <summary>获取当前日志文件路径</summary>
        string GetCurrentLogFile();
    }
}
