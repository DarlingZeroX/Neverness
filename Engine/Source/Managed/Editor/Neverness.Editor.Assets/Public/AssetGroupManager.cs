using System.Text;
using System.Text.Json;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产分组管理器。
///
/// 管理 AssetGroup 的增删改查、持久化。
/// 分组配置存储在：Project/Assets/AssetGroups.json
/// </summary>
public sealed class AssetGroupManager
{
    private readonly Dictionary<string, AssetGroup> _groups = new(StringComparer.OrdinalIgnoreCase);
    private readonly object _lock = new();
    private string _configPath = string.Empty;

    /// <summary>所有分组。</summary>
    public IReadOnlyList<AssetGroup> Groups
    {
        get { lock (_lock) return _groups.Values.ToList(); }
    }

    /// <summary>分组数量。</summary>
    public int Count
    {
        get { lock (_lock) return _groups.Count; }
    }

    /// <summary>初始化并加载配置。</summary>
    public void Initialize(string assetsRoot)
    {
        lock (_lock)
        {
            _configPath = Path.Combine(assetsRoot, "AssetGroups.json");
            Load();
        }
    }

    /// <summary>创建分组。</summary>
    public AssetGroup CreateGroup(string name)
    {
        lock (_lock)
        {
            if (_groups.TryGetValue(name, out var existing))
                return existing;

            var group = new AssetGroup { Name = name };
            _groups[name] = group;
            return group;
        }
    }

    /// <summary>删除分组。</summary>
    public void DeleteGroup(string name)
    {
        lock (_lock) { _groups.Remove(name); }
    }

    /// <summary>按名称获取分组。</summary>
    public AssetGroup? GetGroup(string name)
    {
        lock (_lock)
        {
            _groups.TryGetValue(name, out var group);
            return group;
        }
    }

    /// <summary>将资产加入分组。</summary>
    public void AddToGroup(string groupName, GUID asset)
    {
        lock (_lock)
        {
            var group = GetOrCreateGroup(groupName);
            group.AddAsset(asset);
        }
    }

    /// <summary>将资产从分组中移除。</summary>
    public void RemoveFromGroup(string groupName, GUID asset)
    {
        lock (_lock)
        {
            if (_groups.TryGetValue(groupName, out var group))
                group.RemoveAsset(asset);
        }
    }

    /// <summary>查询资产所属的所有分组。</summary>
    public IReadOnlyList<AssetGroup> FindGroupsContaining(GUID asset)
    {
        lock (_lock)
        {
            return _groups.Values.Where(g => g.Contains(asset)).ToList();
        }
    }

    /// <summary>按地址查找分组。</summary>
    public AssetGroup? FindByAddress(string address)
    {
        lock (_lock)
        {
            return _groups.Values.FirstOrDefault(g =>
                g.Address.Equals(address, StringComparison.OrdinalIgnoreCase));
        }
    }

    /// <summary>持久化配置。</summary>
    public void Save()
    {
        lock (_lock)
        {
            if (string.IsNullOrEmpty(_configPath))
                return;

            try
            {
                var dir = Path.GetDirectoryName(_configPath);
                if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
                    Directory.CreateDirectory(dir);

                var data = new GroupConfigData();
                foreach (var (_, group) in _groups)
                {
                    data.Groups.Add(new GroupData
                    {
                        Name = group.Name,
                        Address = group.Address,
                        Strategy = group.Strategy.ToString(),
                        IncludeInBuild = group.IncludeInBuild,
                        Labels = group.Labels.ToList(),
                        Assets = group.Assets.Select(g => g.ToUuidString()).ToList()
                    });
                }

                var json = JsonSerializer.Serialize(data, new JsonSerializerOptions { WriteIndented = true });
                File.WriteAllText(_configPath, json, Encoding.UTF8);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[AssetGroupManager] 保存配置失败: {ex.Message}");
            }
        }
    }

    /// <summary>加载配置。</summary>
    public void Load()
    {
        if (string.IsNullOrEmpty(_configPath) || !File.Exists(_configPath))
            return;

        try
        {
            var json = File.ReadAllText(_configPath, Encoding.UTF8);
            var data = JsonSerializer.Deserialize<GroupConfigData>(json);
            if (data == null) return;

            _groups.Clear();
            foreach (var gd in data.Groups)
            {
                var group = new AssetGroup
                {
                    Name = gd.Name,
                    Address = gd.Address,
                    Strategy = Enum.TryParse<BuildStrategy>(gd.Strategy, out var s) ? s : BuildStrategy.PackTogether,
                    IncludeInBuild = gd.IncludeInBuild
                };
                group.Labels.AddRange(gd.Labels);
                foreach (var guidStr in gd.Assets)
                    group.Assets.Add(GUID.Parse(guidStr));

                _groups[gd.Name] = group;
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[AssetGroupManager] 加载配置失败: {ex.Message}");
        }
    }

    private AssetGroup GetOrCreateGroup(string name)
    {
        if (!_groups.TryGetValue(name, out var group))
        {
            group = new AssetGroup { Name = name };
            _groups[name] = group;
        }
        return group;
    }

    /* ========== JSON 数据模型 ========== */

    private sealed class GroupConfigData
    {
        public List<GroupData> Groups { get; set; } = new();
    }

    private sealed class GroupData
    {
        public string Name { get; set; } = string.Empty;
        public string Address { get; set; } = string.Empty;
        public string Strategy { get; set; } = "PackTogether";
        public bool IncludeInBuild { get; set; } = true;
        public List<string> Labels { get; set; } = new();
        public List<string> Assets { get; set; } = new();
    }
}
