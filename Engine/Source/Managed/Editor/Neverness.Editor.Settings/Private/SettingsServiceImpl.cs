using Avalonia.Controls;
using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Private;

/// <summary>
/// 设置服务实现——协调 Registry、Storage 和 UI。
/// UI 方法（ShowXxxWindow）为 internal，由 SettingsModule 菜单命令调用。
/// </summary>
internal sealed class SettingsServiceImpl : ISettingsService
{
    private readonly SettingsRegistry _registry;
    private readonly SettingsStorage _storage;
    private readonly Dictionary<string, ISettingsPageProvider> _pageProviders = new(StringComparer.Ordinal);

    // 两个独立的窗口实例（单例，关闭后可重新打开）
    private Window? _projectSettingsWindow;
    private Window? _preferencesWindow;

    public SettingsServiceImpl(SettingsRegistry registry, SettingsStorage storage)
    {
        _registry = registry;
        _storage = storage;
    }

    // ── 查询（公共接口） ──

    public IReadOnlyList<SettingsTable> GetAllTables()
    {
        return _registry.GetAll().Select(r => r.Table).ToList();
    }

    public SettingsTable? GetTable(string tableId)
    {
        return _registry.GetTable(tableId);
    }

    public T? GetTable<T>() where T : SettingsTable
    {
        return _registry.GetAll()
            .Select(r => r.Table)
            .OfType<T>()
            .FirstOrDefault();
    }

    public IReadOnlyList<SettingsTable> GetTablesByScope(SettingsScope scope)
    {
        return _registry.GetAll()
            .Where(r => r.Table.Scope == scope)
            .Select(r => r.Table)
            .ToList();
    }

    public ISettingsDescriptor? GetDescriptor(string tableId)
    {
        return _registry.GetDescriptor(tableId);
    }

    // ── 注册 ──

    public void Register(SettingsTable table, ISettingsDescriptor descriptor)
    {
        _registry.Register(table, descriptor);
    }

    // ── 持久化 ──

    public void SaveAll()
    {
        foreach (var reg in _registry.GetAll())
        {
            _storage.Save(reg.Table);
        }
    }

    public void Save(string tableId)
    {
        var table = _registry.GetTable(tableId);
        if (table != null)
        {
            _storage.Save(table);
        }
    }

    // ── 事件 ──

    public event Action<string, string>? SettingChanged;

    /// <summary>触发设置变更事件。</summary>
    internal void RaiseSettingChanged(string tableId, string fieldName)
    {
        SettingChanged?.Invoke(tableId, fieldName);
    }

    // ── 内部方法（SettingsModule / SettingsWindow 调用） ──

    /// <summary>加载所有设置表。</summary>
    internal void LoadAll()
    {
        foreach (var reg in _registry.GetAll())
        {
            LoadTable(reg.Table);
        }
    }

    /// <summary>加载单个设置表。</summary>
    internal void Load(string tableId)
    {
        var table = _registry.GetTable(tableId);
        if (table != null)
        {
            LoadTable(table);
        }
    }

    /// <summary>注册自定义页面提供者。</summary>
    internal void RegisterPageProvider(ISettingsPageProvider provider)
    {
        _pageProviders[provider.TableId] = provider;
    }

    /// <summary>获取自定义页面提供者。</summary>
    internal ISettingsPageProvider? GetPageProvider(string tableId)
    {
        return _pageProviders.TryGetValue(tableId, out var provider) ? provider : null;
    }

    /// <summary>打开项目设置窗口。</summary>
    internal void ShowProjectSettingsWindow()
    {
        if (_projectSettingsWindow != null)
        {
            _projectSettingsWindow.Activate();
            return;
        }

        _projectSettingsWindow = new Views.SettingsWindow(this, SettingsScope.Project);
        _projectSettingsWindow.Title = "Project Settings";
        _projectSettingsWindow.Closed += (_, _) => _projectSettingsWindow = null;
        _projectSettingsWindow.Show();
    }

    /// <summary>打开用户偏好窗口。</summary>
    internal void ShowPreferencesWindow()
    {
        if (_preferencesWindow != null)
        {
            _preferencesWindow.Activate();
            return;
        }

        _preferencesWindow = new Views.SettingsWindow(this, SettingsScope.User);
        _preferencesWindow.Title = "Preferences";
        _preferencesWindow.Closed += (_, _) => _preferencesWindow = null;
        _preferencesWindow.Show();
    }

    /// <summary>获取所有注册条目（供 SettingsWindow 使用）。</summary>
    internal IReadOnlyList<SettingsRegistry.Registration> GetRegistrations()
    {
        return _registry.GetAll();
    }

    /// <summary>按范围获取注册条目（供 SettingsWindow 使用）。</summary>
    internal IReadOnlyList<SettingsRegistry.Registration> GetRegistrationsByScope(SettingsScope scope)
    {
        return _registry.GetAll()
            .Where(r => r.Table.Scope == scope)
            .ToList();
    }

    /// <summary>获取所有分类。</summary>
    internal IReadOnlyList<string> GetCategories()
    {
        return _registry.GetCategories();
    }

    // ── 私有方法 ──

    /// <summary>加载单个设置表。</summary>
    private void LoadTable(SettingsTable table)
    {
        var json = _storage.LoadJson(table.TableId);
        if (json != null)
        {
            table.LoadFromJson(json);
        }
    }
}
