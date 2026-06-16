using System.Text;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// Lua 脚本导入器。
///
/// 支持格式：.lua
///
/// 导入设置：
///   compileBytecode — true | false（默认 false，是否编译为 Lua 字节码）
///   stripComments    — true | false（默认 false，是否去除注释）
///   stripDebugInfo   — true | false（默认 true，编译字节码时去除调试信息）
/// </summary>
[AssetImporter(".lua")]
public class LuaScriptImporter : ISettingsAwareImporter
{
    public string[] SupportedExtensions => new[] { ".lua" };
    public string DisplayName => "Lua Script Importer";

    public ImportResult Import(AssetImportContext context)
    {
        try
        {
            var result = ImportResult.Ok(context.AssetGuid, AssetTypeId.LuaScript);

            var sourceText = File.ReadAllText(context.SourceAssetPath.FullPath, Encoding.UTF8);

            var compileBytecode = context.GetSettingBool("compileBytecode", false);
            var stripComments = context.GetSettingBool("stripComments", false);

            byte[] data;

            if (compileBytecode)
            {
                /* TODO: 调用 Lua 编译器生成字节码 */
                /* 当前降级为源文本 */
                data = Encoding.UTF8.GetBytes(sourceText);
            }
            else if (stripComments)
            {
                var stripped = StripComments(sourceText);
                data = Encoding.UTF8.GetBytes(stripped);
            }
            else
            {
                data = Encoding.UTF8.GetBytes(sourceText);
            }

            result.Blobs.Add(new ImportedBlob
            {
                BlobType = AssetTypeId.BlobType.Data,
                Data = data
            });

            return result;
        }
        catch (Exception ex)
        {
            return ImportResult.Fail($"Lua 脚本导入异常: {ex.Message}");
        }
    }

    public Dictionary<string, string> GetDefaultSettings() => new()
    {
        ["compileBytecode"] = "false",
        ["stripComments"] = "false",
        ["stripDebugInfo"] = "true"
    };

    public bool ValidateSettings(Dictionary<string, string> settings, out string? errorMessage)
    {
        errorMessage = null;
        return true;
    }

    /// <summary>简单的 Lua 注释去除（不处理字符串字面量中的 --）。</summary>
    private static string StripComments(string source)
    {
        var lines = source.Split('\n');
        var sb = new StringBuilder();

        bool inBlockComment = false;

        foreach (var rawLine in lines)
        {
            var line = rawLine;

            if (inBlockComment)
            {
                var endIdx = line.IndexOf("]]", StringComparison.Ordinal);
                if (endIdx >= 0)
                {
                    line = line[(endIdx + 2)..];
                    inBlockComment = false;
                }
                else
                {
                    continue;
                }
            }

            /* 处理块注释开始 */
            var blockStart = line.IndexOf("--[[", StringComparison.Ordinal);
            if (blockStart >= 0)
            {
                var beforeBlock = line[..blockStart];
                var afterStart = line[(blockStart + 4)..];
                var blockEnd = afterStart.IndexOf("]]", StringComparison.Ordinal);

                if (blockEnd >= 0)
                {
                    line = beforeBlock + afterStart[(blockEnd + 2)..];
                }
                else
                {
                    line = beforeBlock;
                    inBlockComment = true;
                }
            }
            else
            {
                /* 处理行注释 */
                var lineCommentIdx = line.IndexOf("--", StringComparison.Ordinal);
                if (lineCommentIdx >= 0)
                    line = line[..lineCommentIdx];
            }

            if (!string.IsNullOrWhiteSpace(line))
                sb.AppendLine(line.TrimEnd());
        }

        return sb.ToString().TrimEnd();
    }
}
