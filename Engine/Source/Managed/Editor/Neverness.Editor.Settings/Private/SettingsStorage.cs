using Neverness.Runtime.Settings;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Settings.Private;

/// <summary>
/// 设置持久化层——通过 VFS 读写 JSON 文件。
/// 每个设置表一个 JSON 文件，存储在 /projectSettings/Settings/ 目录下。
///
/// 文件布局（VFS 路径）：
///   /projectSettings/Settings/{tableId}.json
/// </summary>
internal sealed class SettingsStorage
{
    /// <summary>设置文件的 VFS 基础路径。</summary>
    private static string SettingsVfsBase => ProjectPaths.ProjectSettings.Combine("Settings").FullPath;

    /// <summary>保存设置表到 JSON 文件。</summary>
    public void Save(SettingsTable table)
    {
        try
        {
            var vfsPath = GetVfsPath(table.TableId);
            if (string.IsNullOrEmpty(vfsPath)) return;

            var json = SettingsSerializer.Save(table);
            VFSService.WriteText(vfsPath, json);
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
            var vfsPath = GetVfsPath(tableId);
            if (string.IsNullOrEmpty(vfsPath)) return null;

            return VFSService.ReadText(vfsPath);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[SettingsStorage] 加载 '{tableId}' 失败: {ex.Message}");
            return null;
        }
    }

    /// <summary>获取设置文件的 VFS 路径。</summary>
    private static string? GetVfsPath(string tableId)
    {
        var safeFileName = string.Join("_", tableId.Split(Path.GetInvalidFileNameChars()));
        return $"{SettingsVfsBase}/{safeFileName}.json";
    }
}
