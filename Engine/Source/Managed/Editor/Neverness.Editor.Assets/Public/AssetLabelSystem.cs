using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// 资产标签系统。
///
/// 提供字符串标签与资产 GUID 之间的双向映射。
/// 复用 EditorAssetDatabase 的 LabelIndex 实现，本类提供更语义化的 API。
///
/// 标签用途：
///   - Addressable 按标签批量加载
///   - 编辑器筛选/搜索
///   - 资产分类管理
/// </summary>
public static class AssetLabelSystem
{
    /// <summary>为资产添加标签。</summary>
    public static void AddLabel(GUID asset, string label)
    {
        EditorAssetDatabase.AddLabel(asset, label);
    }

    /// <summary>移除资产标签。</summary>
    public static void RemoveLabel(GUID asset, string label)
    {
        EditorAssetDatabase.RemoveLabel(asset, label);
    }

    /// <summary>获取资产的所有标签。</summary>
    public static IReadOnlyList<string> GetLabels(GUID asset)
    {
        return EditorAssetDatabase.GetLabels(asset);
    }

    /// <summary>按标签查询所有资产 GUID。</summary>
    public static IReadOnlyList<GUID> FindByLabel(string label)
    {
        return EditorAssetDatabase.FindAssetsByLabel(label);
    }

    /// <summary>资产是否包含指定标签。</summary>
    public static bool HasLabel(GUID asset, string label)
    {
        return EditorAssetDatabase.GetLabels(asset).Contains(label, StringComparer.OrdinalIgnoreCase);
    }

    /// <summary>清除资产的所有标签。</summary>
    public static void ClearLabels(GUID asset)
    {
        var labels = EditorAssetDatabase.GetLabels(asset);
        foreach (var label in labels)
            EditorAssetDatabase.RemoveLabel(asset, label);
    }

    /// <summary>获取系统中所有使用过的标签。</summary>
    public static IReadOnlyList<string> GetAllLabels()
    {
        /* 通过 FindAssetsByLabel 无法获取全量标签列表，
         * 需要 EditorAssetDatabase 暴露 LabelIndex，
         * 当前降级实现：遍历所有资产 */
        var labels = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (var guid in EditorAssetDatabase.AllAssets)
        {
            foreach (var label in EditorAssetDatabase.GetLabels(guid))
                labels.Add(label);
        }
        return labels.ToList();
    }
}
