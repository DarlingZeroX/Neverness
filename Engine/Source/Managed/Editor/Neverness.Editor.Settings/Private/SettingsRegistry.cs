using Neverness.Runtime.Settings;

namespace Neverness.Editor.Settings.Private;

/// <summary>
/// 设置表注册中心——管理所有 SettingsTable 和对应的 Descriptor。
/// 内部使用字典按 tableId 索引，支持按分类查询。
/// </summary>
internal sealed class SettingsRegistry
{
    /// <summary>注册条目。</summary>
    internal sealed record Registration(SettingsTable Table, ISettingsDescriptor Descriptor);

    private readonly Dictionary<string, Registration> _tables = new(StringComparer.Ordinal);
    private readonly List<Registration> _ordered = new();

    /// <summary>已注册的设置表数量。</summary>
    public int Count => _tables.Count;

    /// <summary>注册设置表。</summary>
    public void Register(SettingsTable table, ISettingsDescriptor descriptor)
    {
        ArgumentNullException.ThrowIfNull(table);
        ArgumentNullException.ThrowIfNull(descriptor);

        if (_tables.ContainsKey(table.TableId))
        {
            Console.WriteLine($"[SettingsRegistry] 设置表 '{table.TableId}' 已注册，跳过重复注册。");
            return;
        }

        var reg = new Registration(table, descriptor);
        _tables[table.TableId] = reg;
        _ordered.Add(reg);
    }

    /// <summary>按 ID 获取设置表。</summary>
    public SettingsTable? GetTable(string tableId)
    {
        return _tables.TryGetValue(tableId, out var reg) ? reg.Table : null;
    }

    /// <summary>按 ID 获取描述符。</summary>
    public ISettingsDescriptor? GetDescriptor(string tableId)
    {
        return _tables.TryGetValue(tableId, out var reg) ? reg.Descriptor : null;
    }

    /// <summary>获取所有注册条目（按注册顺序）。</summary>
    public IReadOnlyList<Registration> GetAll() => _ordered;

    /// <summary>按分类获取注册条目。</summary>
    public IReadOnlyList<Registration> GetByCategory(string category)
    {
        return _ordered
            .Where(r => string.Equals(r.Descriptor.Category, category, StringComparison.Ordinal))
            .ToList();
    }

    /// <summary>获取所有分类（去重，按首次出现顺序）。</summary>
    public IReadOnlyList<string> GetCategories()
    {
        return _ordered
            .Select(r => r.Descriptor.Category)
            .Where(c => !string.IsNullOrEmpty(c))
            .Distinct(StringComparer.Ordinal)
            .Cast<string>()
            .ToList();
    }
}
