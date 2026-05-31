using NevernessLauncher.Contracts;
using NevernessLauncher.Core.Constants;
using System;
using System.IO;

namespace NevernessLauncher.Services
{
    /// <summary>
    /// 日志服务实现
    /// </summary>
    public class LogService : ILogService
    {
        private readonly string _logDirectory;
        private readonly string _currentLogFile;
        private readonly object _lock = new();

        public LogService()
        {
            _logDirectory = Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                PathConstants.ConfigDirectoryName,
                PathConstants.LauncherDataDirectoryName,
                PathConstants.LogsDirectoryName
            );

            if (!Directory.Exists(_logDirectory))
            {
                Directory.CreateDirectory(_logDirectory);
            }

            var date = DateTime.Now.ToString("yyyy-MM-dd");
            _currentLogFile = Path.Combine(_logDirectory, $"{AppConstants.LogFilePrefix}{date}{AppConstants.LogFileExtension}");
        }

        /// <summary>记录 Trace 级别日志</summary>
        public void LogTrace(string message) => Log("TRACE", message);

        /// <summary>记录 Debug 级别日志</summary>
        public void LogDebug(string message) => Log("DEBUG", message);

        /// <summary>记录 Info 级别日志</summary>
        public void LogInfo(string message) => Log("INFO ", message);

        /// <summary>记录 Warning 级别日志</summary>
        public void LogWarning(string message) => Log("WARN ", message);

        /// <summary>记录 Error 级别日志</summary>
        public void LogError(string message, Exception? exception = null)
        {
            Log("ERROR", message);
            if (exception != null)
            {
                Log("ERROR", exception.ToString());
            }
        }

        /// <summary>记录 Critical 级别日志</summary>
        public void LogCritical(string message, Exception? exception = null)
        {
            Log("CRIT ", message);
            if (exception != null)
            {
                Log("CRIT ", exception.ToString());
            }
        }

        /// <summary>获取日志目录路径</summary>
        public string GetLogDirectory() => _logDirectory;

        /// <summary>获取当前日志文件路径</summary>
        public string GetCurrentLogFile() => _currentLogFile;

        private void Log(string level, string message)
        {
            var timestamp = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
            var logEntry = $"[{timestamp}] [{level}] {message}";

            lock (_lock)
            {
                try
                {
                    File.AppendAllText(_currentLogFile, logEntry + Environment.NewLine);
                }
                catch
                {
                    // 日志写入失败时静默处理，避免递归异常
                }
            }

            // 同时输出到调试输出
            System.Diagnostics.Debug.WriteLine(logEntry);
        }
    }
}
