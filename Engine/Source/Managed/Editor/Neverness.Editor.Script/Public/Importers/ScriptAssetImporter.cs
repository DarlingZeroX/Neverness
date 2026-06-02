// ============================================================================
// ScriptAssetImporter.cs - C# 脚本资产导入器
// ============================================================================
// 导入 .cs 文件为 ScriptAsset 类型，注册到 AssetDatabase。
// 编译流程独立，由 ScriptCompileQueue 异步处理。
// ============================================================================

using System.Text;
using Neverness.Editor.Assets;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Script.Public;

/// <summary>
/// C# 脚本导入器。
///
/// 支持格式：.cs
///
/// 导入设置：
///   targetFramework — net10.0（默认，目标框架）
///   enableNullable   — true | false（默认 true，是否启用 Nullable）
/// </summary>
[AssetImporter(".cs")]
public class ScriptAssetImporter : ISettingsAwareImporter
{
    // ========================================================================
    // IAssetImporter
    // ========================================================================

    /// <inheritdoc/>
    public string[] SupportedExtensions => new[] { ".cs" };

    /// <inheritdoc/>
    public string DisplayName => "C# Script Importer";

    /// <inheritdoc/>
    public ImportResult Import(AssetImportContext context)
    {
        try
        {
            // 1. 读取源文件
            var sourcePath = context.SourceAssetPath.FullPath;
            if (!File.Exists(sourcePath))
            {
                return ImportResult.Fail($"C# 脚本文件不存在: {sourcePath}");
            }

            var sourceText = File.ReadAllText(sourcePath, Encoding.UTF8);

            // 2. 基本语法验证
            if (!ValidateSyntax(sourceText, out var error))
            {
                return ImportResult.Fail($"C# 脚本语法验证失败: {error}");
            }

            // 3. 创建导入结果
            var result = ImportResult.Ok(context.AssetGuid, AssetTypeId.CSharpScript);

            // 4. 存储源文本
            result.Blobs.Add(new ImportedBlob
            {
                BlobType = AssetTypeId.BlobType.Data,
                Data = Encoding.UTF8.GetBytes(sourceText)
            });

            // 5. 触发编译队列（延迟执行）
            ScriptCompileQueue.Enqueue(sourcePath);

            return result;
        }
        catch (Exception ex)
        {
            return ImportResult.Fail($"C# 脚本导入异常: {ex.Message}");
        }
    }

    // ========================================================================
    // ISettingsAwareImporter
    // ========================================================================

    /// <inheritdoc/>
    public Dictionary<string, string> GetDefaultSettings() => new()
    {
        ["targetFramework"] = "net10.0",
        ["enableNullable"] = "true"
    };

    /// <inheritdoc/>
    public bool ValidateSettings(Dictionary<string, string> settings, out string? errorMessage)
    {
        errorMessage = null;
        return true;
    }

    // ========================================================================
    // 内部方法
    // ========================================================================

    /// <summary>基本语法验证（检查类声明等）。</summary>
    private static bool ValidateSyntax(string source, out string? error)
    {
        error = null;

        // 基本检查：不为空
        if (string.IsNullOrWhiteSpace(source))
        {
            error = "源文件为空";
            return false;
        }

        // 基本检查：包含 class 关键字（简单验证）
        if (!source.Contains("class", StringComparison.Ordinal))
        {
            error = "源文件中未找到 class 声明";
            return false;
        }

        return true;
    }
}
