// ============================================================================
// ScriptAssetOpener.cs - 脚本资产打开器
// ============================================================================
// 支持双击 .cs 文件在外部编辑器中打开。
// ============================================================================

using System.Diagnostics;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Script.Public;

/// <summary>
/// 脚本资产打开器：支持双击 .cs 文件在外部编辑器中打开。
/// </summary>
public sealed class ScriptAssetOpener
{
    // ========================================================================
    // 常量
    // ========================================================================

    /// <summary>支持的文件扩展名。</summary>
    private static readonly string[] SupportedExtensions = { ".cs" };

    // ========================================================================
    // 公共方法
    // ========================================================================

    /// <summary>
    /// 检查是否可以打开指定的资产。
    /// </summary>
    /// <param name="assetPath">资产路径。</param>
    /// <returns>是否可以打开。</returns>
    public static bool CanOpen(string assetPath)
    {
        if (string.IsNullOrWhiteSpace(assetPath))
            return false;

        var ext = Path.GetExtension(assetPath).ToLowerInvariant();
        return SupportedExtensions.Contains(ext);
    }

    /// <summary>
    /// 打开脚本资产。
    /// </summary>
    /// <param name="assetPath">资产路径。</param>
    /// <returns>是否成功打开。</returns>
    public static bool Open(string assetPath)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(assetPath);

        if (!CanOpen(assetPath))
            return false;

        try
        {
            // 获取源文件路径
            var sourcePath = ResolveSourcePath(assetPath);
            if (sourcePath is null || !File.Exists(sourcePath))
            {
                return false;
            }

            // 使用系统默认编辑器打开
            return OpenWithDefaultEditor(sourcePath);
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Failed to open script asset: {ex.Message}");
            return false;
        }
    }

    /// <summary>
    /// 在指定编辑器中打开脚本资产。
    /// </summary>
    /// <param name="assetPath">资产路径。</param>
    /// <param name="editorPath">编辑器路径。</param>
    /// <returns>是否成功打开。</returns>
    public static bool OpenWithEditor(string assetPath, string editorPath)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(assetPath);
        ArgumentException.ThrowIfNullOrWhiteSpace(editorPath);

        if (!CanOpen(assetPath))
            return false;

        try
        {
            var sourcePath = ResolveSourcePath(assetPath);
            if (sourcePath is null || !File.Exists(sourcePath))
            {
                return false;
            }

            var process = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = editorPath,
                    Arguments = $"\"{sourcePath}\"",
                    UseShellExecute = false,
                    CreateNoWindow = false
                }
            };

            process.Start();
            return true;
        }
        catch (Exception ex)
        {
            System.Diagnostics.Debug.WriteLine($"Failed to open script asset with editor: {ex.Message}");
            return false;
        }
    }

    // ========================================================================
    // 内部方法
    // ========================================================================

    /// <summary>解析源文件路径。</summary>
    private static string? ResolveSourcePath(string assetPath)
    {
        // 如果是相对路径，尝试相对于项目目录解析
        if (Path.IsPathRooted(assetPath))
        {
            return assetPath;
        }

        // TODO: 从 AssetDatabase 获取实际路径
        return assetPath;
    }

    /// <summary>使用系统默认编辑器打开文件。</summary>
    private static bool OpenWithDefaultEditor(string filePath)
    {
        try
        {
            var process = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = filePath,
                    UseShellExecute = true,
                    Verb = "open"
                }
            };

            process.Start();
            return true;
        }
        catch
        {
            return false;
        }
    }
}
