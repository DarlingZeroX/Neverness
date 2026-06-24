using Avalonia.Controls;
using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Private;

/// <summary>
/// 设置服务实现——协调 Registry、Storage 和 UI。
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

    // ── 查询 ──

    public IReadOnlyList<SettingsTable> GetAllTables()
    {
        return _registry.GetAll().Select(r => r.Table).ToList();
    }

    public SettingsTable? GetTable(string tableId)
    {
        return _registry.GetTable(tableId);
    }

    public IReadOnlyList<SettingsTable> GetTablesByCategory(string category)
    {
        return _registry.GetByCategory(category).Select(r => r.Table).ToList();
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

    public void RegisterPageProvider(ISettingsPageProvider provider)
    {
        _pageProviders[provider.TableId] = provider;
    }

    // ── 持久化 ──

    public void LoadAll()
    {
        foreach (var reg in _registry.GetAll())
        {
            LoadTable(reg.Table);
        }
    }

    public void SaveAll()
    {
        foreach (var reg in _registry.GetAll())
        {
            _storage.Save(reg.Table);
        }
    }

    public void Load(string tableId)
    {
        var table = _registry.GetTable(tableId);
        if (table != null)
        {
            LoadTable(table);
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

    // ── UI ──

    public void ShowProjectSettingsWindow()
    {
        // 如果窗口已打开且未关闭，激活它
        if (_projectSettingsWindow != null)
        {
            _projectSettingsWindow.Activate();
            return;
        }

        // 创建项目设置窗口（只显示 Project 范围的设置表）
        _projectSettingsWindow = new Views.SettingsWindow(this, SettingsScope.Project);
        _projectSettingsWindow.Title = "Project Settings";
        _projectSettingsWindow.Closed += (_, _) => _projectSettingsWindow = null;
        _projectSettingsWindow.Show();
    }

    public void ShowPreferencesWindow()
    {
        // 如果窗口已打开且未关闭，激活它
        if (_preferencesWindow != null)
        {
            _preferencesWindow.Activate();
            return;
        }

        // 创建用户偏好窗口（只显示 User 范围的设置表）
        _preferencesWindow = new Views.SettingsWindow(this, SettingsScope.User);
        _preferencesWindow.Title = "Preferences";
        _preferencesWindow.Closed += (_, _) => _preferencesWindow = null;
        _preferencesWindow.Show();
    }

    // ── 事件 ──

    public event Action<string, string>? SettingChanged;

    /// <summary>触发设置变更事件。</summary>
    internal void RaiseSettingChanged(string tableId, string fieldName)
    {
        SettingChanged?.Invoke(tableId, fieldName);
    }

    /// <summary>获取自定义页面提供者。</summary>
    internal ISettingsPageProvider? GetPageProvider(string tableId)
    {
        return _pageProviders.TryGetValue(tableId, out var provider) ? provider : null;
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

    // ── 内部方法 ──

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
