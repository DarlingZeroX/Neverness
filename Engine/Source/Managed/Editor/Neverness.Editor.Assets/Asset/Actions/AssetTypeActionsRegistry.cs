namespace Neverness.Editor.Assets.AssetActions;

/// <summary>
/// 资产类型操作注册表——存储每种资产类型的编辑器行为元数据。
/// </summary>
public sealed class AssetTypeActionsRegistry
{
    public static AssetTypeActionsRegistry Instance { get; } = new();

    private readonly Dictionary<string, AssetTypeActions> _actions = new(StringComparer.OrdinalIgnoreCase);

    /// <summary>注册一种资产类型操作。</summary>
    public void Register(AssetTypeActions actions)
    {
        ArgumentNullException.ThrowIfNull(actions);
        _actions[actions.FileExtension] = actions;
    }

    /// <summary>按扩展名查找资产类型操作。</summary>
    public AssetTypeActions? FindByExtension(string extension)
    {
        _actions.TryGetValue(extension, out var actions);
        return actions;
    }

    /// <summary>获取所有已注册的资产类型操作。</summary>
    public IReadOnlyDictionary<string, AssetTypeActions> All => _actions;
}
