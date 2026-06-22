using System.Text.Json;
using Neverness.Editor.Core.Public;
using Neverness.Editor.ProjectSystem.Public;
using Neverness.Runtime.VFS.Public;

namespace Neverness.Editor.Core.Private;

/// <summary>
/// 编辑器偏好设置服务实现——持久化到 projectSettings/EditorPrefs.json。
/// </summary>
public sealed class PreferencesServiceImpl : IPreferencesService
{
    private static readonly string PrefsFileName = "EditorPrefs.json";

    private IDEPreference _preferredIDE = IDEPreference.VisualStudio;

    /// <summary>获取或设置首选 IDE。</summary>
    public IDEPreference PreferredIDE
    {
        get => _preferredIDE;
        set => _preferredIDE = value;
    }

    /// <summary>保存设置到 JSON 文件。</summary>
    public void Save()
    {
        try
        {
            var settingsPath = GetPrefsFilePath();
            if (string.IsNullOrEmpty(settingsPath))
            {
                Console.WriteLine("[PreferencesService] 无法获取设置路径，保存失败");
                return;
            }

            var dir = Path.GetDirectoryName(settingsPath);
            if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                Directory.CreateDirectory(dir);

            var data = new PreferencesData
            {
                PreferredIDE = _preferredIDE.ToString()
            };

            var json = JsonSerializer.Serialize(data, new JsonSerializerOptions
            {
                WriteIndented = true
            });

            File.WriteAllText(settingsPath, json);
            Console.WriteLine($"[PreferencesService] 设置已保存: {settingsPath}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[PreferencesService] 保存设置失败: {ex.Message}");
        }
    }

    /// <summary>从 JSON 文件加载设置。</summary>
    public void Load()
    {
        try
        {
            var settingsPath = GetPrefsFilePath();
            if (string.IsNullOrEmpty(settingsPath) || !File.Exists(settingsPath))
            {
                Console.WriteLine("[PreferencesService] 设置文件不存在，使用默认值");
                return;
            }

            var json = File.ReadAllText(settingsPath);
            var data = JsonSerializer.Deserialize<PreferencesData>(json);

            if (data != null && !string.IsNullOrEmpty(data.PreferredIDE))
            {
                if (Enum.TryParse<IDEPreference>(data.PreferredIDE, out var pref))
                {
                    _preferredIDE = pref;
                    Console.WriteLine($"[PreferencesService] 加载设置: PreferredIDE={_preferredIDE}");
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[PreferencesService] 加载设置失败: {ex.Message}");
        }
    }

    /// <summary>获取设置文件的物理路径。</summary>
    private static string? GetPrefsFilePath()
    {
        var settingsDir = VFS.GetAbsolutePath(ProjectPaths.Settings.FullPath);
        if (string.IsNullOrEmpty(settingsDir))
            return null;

        return Path.Combine(settingsDir, PrefsFileName);
    }

    /// <summary>JSON 序列化模型。</summary>
    private sealed class PreferencesData
    {
        public string PreferredIDE { get; set; } = IDEPreference.VisualStudio.ToString();
    }
}
