// ============================================================================
// Debug.cs - 调试日志 API
// ============================================================================
// 调试日志 API，封装 Native 日志系统。
// ============================================================================

using System.Runtime.CompilerServices;

namespace Neverness.Gameplay;

/// <summary>
/// 调试日志 API。
/// </summary>
public static class Debug
{
    // ========================================================================
    // 日志方法
    // ========================================================================

    /// <summary>信息日志。</summary>
    /// <param name="message">日志消息。</param>
    public static void Log(object? message)
    {
        System.Diagnostics.Debug.WriteLine($"[INFO] {message}");
        Console.WriteLine($"[INFO] {message}");
    }

    /// <summary>警告日志。</summary>
    /// <param name="message">日志消息。</param>
    public static void LogWarning(object? message)
    {
        System.Diagnostics.Debug.WriteLine($"[WARN] {message}");
        Console.ForegroundColor = ConsoleColor.Yellow;
        Console.WriteLine($"[WARN] {message}");
        Console.ResetColor();
    }

    /// <summary>错误日志。</summary>
    /// <param name="message">日志消息。</param>
    public static void LogError(object? message)
    {
        System.Diagnostics.Debug.WriteLine($"[ERROR] {message}");
        Console.ForegroundColor = ConsoleColor.Red;
        Console.WriteLine($"[ERROR] {message}");
        Console.ResetColor();
    }

    /// <summary>断言。</summary>
    /// <param name="condition">断言条件。</param>
    /// <param name="message">断言失败时的消息。</param>
    public static void Assert(bool condition, string? message = null)
    {
        if (!condition)
        {
            var msg = message ?? "Assertion failed";
            System.Diagnostics.Debug.Fail(msg);
            LogError($"Assert failed: {msg}");
        }
    }
}
