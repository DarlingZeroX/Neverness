using Neverness.Editor.ProjectSystem.Public;
using Neverness.Runtime.Settings;
using Neverness.Runtime.VFS.Public;

namespace Neverness.Editor.Settings.Private;

/// <summary>
/// 设置持久化层——使用 VFS 解析路径，JSON 序列化。
/// 每个设置表一个 JSON 文件，存储在 projectSettings/Settings/ 目录下。
///
/// 文件布局：
///   {projectRoot}/projectSettings/Settings/{tableId}.json
///   例：projectSettings/Settings/graphics.json
/// </summary>
internal sealed class SettingsStorage
{
    private const string SettingsSubDirectory = "Settings";

    /// <summary>保存设置表到 JSON 文件。</summary>
    public void Save(SettingsTable table)
    {
        try
        {
            var filePath = GetFilePath(table.TableId);
            if (string.IsNullOrEmpty(filePath))
            {
                Console.WriteLine($"[SettingsStorage] 无法获取路径，保存 '{table.TableId}' 失败。");
                return;
            }

            var dir = Path.GetDirectoryName(filePath);
            if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                Directory.CreateDirectory(dir);

            var json = SettingsSerializer.Save(table);
            File.WriteAllText(filePath, json);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[SettingsStorage] 保存 '{table.TableId}' 失败: {ex.Message}");
        }
    }

    /// <summary>从 JSON 文件加载设置表。文件不存在时返回 null。</summary>
    public string? LoadJson(string tableId)
    {
        try
        {
            var filePath = GetFilePath(tableId);
            if (string.IsNullOrEmpty(filePath) || !File.Exists(filePath))
                return null;

            return File.ReadAllText(filePath);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[SettingsStorage] 加载 '{tableId}' 失败: {ex.Message}");
            return null;
        }
    }

    /// <summary>检查设置文件是否存在。</summary>
    public bool Exists(string tableId)
    {
        var filePath = GetFilePath(tableId);
        return !string.IsNullOrEmpty(filePath) && File.Exists(filePath);
    }

    /// <summary>获取设置文件的物理路径。</summary>
    private static string? GetFilePath(string tableId)
    {
        // 清理 tableId 中的非法文件名字符
        var safeFileName = string.Join("_", tableId.Split(Path.GetInvalidFileNameChars()));
        var settingsDir = VFS.GetAbsolutePath(ProjectPaths.Settings.FullPath);
        if (string.IsNullOrEmpty(settingsDir))
            return null;

        return Path.Combine(settingsDir, SettingsSubDirectory, $"{safeFileName}.json");
    }
}
