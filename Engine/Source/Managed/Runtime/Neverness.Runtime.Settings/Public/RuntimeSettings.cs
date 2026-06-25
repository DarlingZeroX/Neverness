using System.ComponentModel;
using Neverness.Runtime.Settings.Descriptors;
using Neverness.Runtime.VFS;

namespace Neverness.Runtime.Settings;

/// <summary>
/// 运行时设置的全局静态访问器。
/// 通过 VFS 读写设置文件，运行时可读写，修改即时生效。
///
/// 用法：
/// <code>
/// // 初始化（VFS 就绪后调用一次）
/// RuntimeSettings.Initialize();
///
/// // 读取
/// bool vsync = RuntimeSettings.Graphics.VSync;
/// float volume = RuntimeSettings.Audio.MasterVolume;
///
/// // 写入（触发 PropertyChanged + SettingsChanged）
/// RuntimeSettings.Graphics.VSync = false;
///
/// // 监听变更
/// RuntimeSettings.SettingsChanged += (tableId, field) => { ... };
/// </code>
/// </summary>
public static class RuntimeSettings
{
    // ── 设置表实例 ──

    /// <summary>图形设置。</summary>
    public static GraphicsSettings Graphics { get; } = new();

    /// <summary>音频设置。</summary>
    public static AudioSettings Audio { get; } = new();

    // ── 变更通知 ──

    /// <summary>
    /// 全局设置变更事件（tableId, fieldName）。
    /// 任意 Runtime 设置表的 PropertyChanged 都会转发到此事件。
    /// </summary>
    public static event Action<string, string>? SettingsChanged;

    // ── 状态 ──

    private static bool _initialized;

    /// <summary>设置文件的 VFS 基础路径（/projectSettings/Settings/）。</summary>
    private static string SettingsVfsBase => ProjectPaths.ProjectSettings.Combine("Settings").FullPath;

    // ── 初始化 ──

    /// <summary>
    /// 初始化运行时设置——通过 VFS 加载 JSON 文件。
    /// 应在 VFS 就绪后调用一次。
    ///
    /// 文件布局（VFS 路径）：
    ///   /projectSettings/Settings/graphics.json
    ///   /projectSettings/Settings/audio.json
    /// </summary>
    public static void Initialize()
    {
        if (_initialized)
        {
            Console.WriteLine("[RuntimeSettings] 已初始化，跳过重复调用。");
            return;
        }

        _initialized = true;

        // 订阅每个设置表的 PropertyChanged，转发到全局 SettingsChanged
        SubscribeTable(Graphics, "graphics");
        SubscribeTable(Audio, "audio");

        // 加载已保存的设置
        ReloadAll();

        Console.WriteLine("[RuntimeSettings] 已初始化。");
    }

    // ── 持久化 ──

    /// <summary>重新加载指定设置表。</summary>
    /// <param name="tableId">设置表 ID（如 "graphics"）。</param>
    public static void Reload(string tableId)
    {
        if (!_initialized)
            return;

        var table = FindTable(tableId);
        if (table == null) return;

        var vfsPath = GetVfsPath(tableId);
        if (string.IsNullOrEmpty(vfsPath)) return;

        try
        {
            var json = VFSService.ReadText(vfsPath);
            if (!string.IsNullOrEmpty(json))
            {
                table.LoadFromJson(json);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[RuntimeSettings] 加载 '{tableId}' 失败: {ex.Message}");
        }
    }

    /// <summary>重新加载所有运行时设置。</summary>
    public static void ReloadAll()
    {
        Reload("graphics");
        Reload("audio");
    }

    /// <summary>保存指定设置表。</summary>
    /// <param name="tableId">设置表 ID（如 "graphics"）。</param>
    public static void Save(string tableId)
    {
        if (!_initialized)
            return;

        var table = FindTable(tableId);
        if (table == null) return;

        var vfsPath = GetVfsPath(tableId);
        if (string.IsNullOrEmpty(vfsPath)) return;

        try
        {
            var json = SettingsSerializer.Save(table);
            VFSService.WriteText(vfsPath, json);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[RuntimeSettings] 保存 '{tableId}' 失败: {ex.Message}");
        }
    }

    /// <summary>保存所有运行时设置。</summary>
    public static void SaveAll()
    {
        Save("graphics");
        Save("audio");
    }

    // ── 内部工具 ──

    /// <summary>订阅设置表的 PropertyChanged，转发到全局 SettingsChanged。</summary>
    private static void SubscribeTable(SettingsTable table, string tableId)
    {
        table.PropertyChanged += (_, e) =>
        {
            if (!string.IsNullOrEmpty(e.PropertyName))
            {
                SettingsChanged?.Invoke(tableId, e.PropertyName);
            }
        };
    }

    /// <summary>按 tableId 查找设置表实例。</summary>
    private static SettingsTable? FindTable(string tableId)
    {
        return tableId switch
        {
            "graphics" => Graphics,
            "audio" => Audio,
            _ => null
        };
    }

    /// <summary>获取设置文件的 VFS 路径。</summary>
    private static string? GetVfsPath(string tableId)
    {
        var safeFileName = string.Join("_", tableId.Split(Path.GetInvalidFileNameChars()));
        return $"{SettingsVfsBase}/{safeFileName}.json";
    }
}
