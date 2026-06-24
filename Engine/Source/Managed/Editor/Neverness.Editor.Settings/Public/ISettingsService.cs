using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings;

/// <summary>
/// 设置服务——管理所有设置表的注册、加载、保存、UI 展示。
/// 通过 IEditorContext.GetService&lt;ISettingsService&gt;() 获取。
/// </summary>
public interface ISettingsService
{
    // ── 查询 ──

    /// <summary>获取所有已注册的设置表。</summary>
    IReadOnlyList<SettingsTable> GetAllTables();

    /// <summary>按 ID 获取设置表。</summary>
    SettingsTable? GetTable(string tableId);

    /// <summary>按分类获取设置表。</summary>
    IReadOnlyList<SettingsTable> GetTablesByCategory(string category);

    /// <summary>按范围获取设置表。</summary>
    IReadOnlyList<SettingsTable> GetTablesByScope(SettingsScope scope);

    /// <summary>获取设置表的描述符。</summary>
    ISettingsDescriptor? GetDescriptor(string tableId);

    // ── 注册 ──

    /// <summary>注册设置表（模块扩展入口）。</summary>
    /// <param name="table">设置表实例。</param>
    /// <param name="descriptor">设置表描述符。</param>
    void Register(SettingsTable table, ISettingsDescriptor descriptor);

    /// <summary>注册自定义页面提供者。</summary>
    void RegisterPageProvider(ISettingsPageProvider provider);

    // ── 持久化 ──

    /// <summary>加载所有设置表。</summary>
    void LoadAll();

    /// <summary>保存所有设置表。</summary>
    void SaveAll();

    /// <summary>加载单个设置表。</summary>
    void Load(string tableId);

    /// <summary>保存单个设置表。</summary>
    void Save(string tableId);

    // ── UI ──

    /// <summary>打开项目设置窗口（Project Settings）。</summary>
    void ShowProjectSettingsWindow();

    /// <summary>打开用户偏好窗口（Preferences）。</summary>
    void ShowPreferencesWindow();

    // ── 事件 ──

    /// <summary>设置变更事件（tableId, fieldName）。</summary>
    event Action<string, string>? SettingChanged;
}
