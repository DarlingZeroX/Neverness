using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings;

/// <summary>
/// 设置服务——管理所有设置表的注册、保存、UI 展示。
/// 通过 CoreModuleImp.Context.TryGetService&lt;ISettingsService&gt;() 获取。
///
/// Runtime 设置通过 RuntimeSettings 静态类直接访问，此接口主要用于：
/// 1. Editor 专属设置（EditorSettings 等）
/// 2. 注册外部模块的设置表
/// 3. 触发 UI 窗口
/// 4. 获取描述符（供 UI 渲染）
/// </summary>
public interface ISettingsService
{
    // ── 查询 ──

    /// <summary>获取所有已注册的设置表。</summary>
    IReadOnlyList<SettingsTable> GetAllTables();

    /// <summary>按 ID 获取设置表。</summary>
    SettingsTable? GetTable(string tableId);

    /// <summary>按类型获取设置表（便捷泛型方法）。</summary>
    T? GetTable<T>() where T : SettingsTable;

    /// <summary>按范围获取设置表。</summary>
    IReadOnlyList<SettingsTable> GetTablesByScope(SettingsScope scope);

    /// <summary>获取设置表的描述符。</summary>
    ISettingsDescriptor? GetDescriptor(string tableId);

    // ── 注册 ──

    /// <summary>注册设置表（模块扩展入口）。</summary>
    /// <param name="table">设置表实例。</param>
    /// <param name="descriptor">设置表描述符。</param>
    void Register(SettingsTable table, ISettingsDescriptor descriptor);

    // ── 持久化 ──

    /// <summary>保存所有设置表。</summary>
    void SaveAll();

    /// <summary>保存单个设置表。</summary>
    void Save(string tableId);

    // ── 事件 ──

    /// <summary>设置变更事件（tableId, fieldName）。</summary>
    event Action<string, string>? SettingChanged;
}
