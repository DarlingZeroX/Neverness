using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Assets;

/// <summary>
/// EditorAssetDatabase — 標籤索引 + 髒標記追蹤。
/// </summary>
public static partial class EditorAssetDatabase
{
    /* === 標籤索引 === */
    private static readonly Dictionary<string, HashSet<string>> s_labelToGuids = new(StringComparer.OrdinalIgnoreCase);
    private static readonly Dictionary<string, HashSet<string>> s_guidToLabels = new();

    /* === 髒標記 === */
    private static readonly HashSet<string> s_dirtyGuids = new();

    /* ======================== 標籤 ======================== */

    /// <summary>為資產添加標籤。</summary>
    public static void AddLabel(GUID guid, string label)
    {
        lock (s_lock)
        {
            if (guid.IsZero || string.IsNullOrWhiteSpace(label)) return;

            var guidKey = guid.ToHexString();

            if (!s_labelToGuids.TryGetValue(label, out var guids))
            {
                guids = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                s_labelToGuids[label] = guids;
            }
            guids.Add(guidKey);

            if (!s_guidToLabels.TryGetValue(guidKey, out var labels))
            {
                labels = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                s_guidToLabels[guidKey] = labels;
            }
            labels.Add(label);
            s_cacheDirty = true;
        }
    }

    /// <summary>移除資產標籤。</summary>
    public static void RemoveLabel(GUID guid, string label)
    {
        lock (s_lock)
        {
            if (guid.IsZero || string.IsNullOrWhiteSpace(label)) return;

            var guidKey = guid.ToHexString();

            if (s_labelToGuids.TryGetValue(label, out var guids))
                guids.Remove(guidKey);

            if (s_guidToLabels.TryGetValue(guidKey, out var labels))
                labels.Remove(label);
            s_cacheDirty = true;
        }
    }

    /// <summary>取得資產的所有標籤。</summary>
    public static IReadOnlyList<string> GetLabels(GUID guid)
    {
        lock (s_lock)
        {
            if (guid.IsZero) return Array.Empty<string>();
            var guidKey = guid.ToHexString();
            return s_guidToLabels.TryGetValue(guidKey, out var labels)
                ? labels.ToList()
                : Array.Empty<string>();
        }
    }

    /// <summary>依標籤查詢資產。</summary>
    public static IReadOnlyList<GUID> FindAssetsByLabel(string label)
    {
        lock (s_lock)
        {
            if (s_labelToGuids.TryGetValue(label, out var guids))
                return guids.Select(g => GUID.Parse(g)).ToList();
            return Array.Empty<GUID>();
        }
    }

    /* ======================== 髒標記 ======================== */

    /// <summary>標記資產為髒。</summary>
    public static void MarkDirty(GUID guid)
    {
        lock (s_lock)
        {
            if (!guid.IsZero)
                s_dirtyGuids.Add(guid.ToHexString());
        }
    }

    /// <summary>資產是否為髒。</summary>
    public static bool IsDirty(GUID guid)
    {
        lock (s_lock) return !guid.IsZero && s_dirtyGuids.Contains(guid.ToHexString());
    }

    /// <summary>清除所有髒標記。</summary>
    public static void ClearDirtyFlags()
    {
        lock (s_lock) s_dirtyGuids.Clear();
    }

    /// <summary>取得所有髒資產。</summary>
    public static IReadOnlyList<GUID> GetDirtyAssets()
    {
        lock (s_lock) return s_dirtyGuids.Select(g => GUID.Parse(g)).ToList();
    }
}
