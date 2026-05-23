namespace Neverness.Editor.Framework.Private.Menu;

/// <summary>
/// 快捷键字符串格式化与标准化。
/// 统一修饰键顺序：Ctrl+Alt+Shift+Key。
/// </summary>
internal static class ShortcutFormatter
{
    /// <summary>标准化快捷键字符串（统一修饰键顺序和大小写）。</summary>
    public static string Normalize(string shortcut)
    {
        if (string.IsNullOrEmpty(shortcut)) return "";

        // 拆分修饰键
        var parts = shortcut.Split('+', StringSplitOptions.TrimEntries);
        var mods = new List<string>(4);
        string key = "";

        foreach (var part in parts)
        {
            switch (part.ToUpperInvariant())
            {
                case "CTRL" or "CONTROL": mods.Add("Ctrl"); break;
                case "ALT": mods.Add("Alt"); break;
                case "SHIFT": mods.Add("Shift"); break;
                default: key = part.ToUpperInvariant(); break;
            }
        }

        // 固定顺序：Ctrl+Alt+Shift+Key
        mods.Sort((a, b) =>
        {
            var order = new Dictionary<string, int> { ["Ctrl"] = 0, ["Alt"] = 1, ["Shift"] = 2 };
            return order.GetValueOrDefault(a, 99).CompareTo(order.GetValueOrDefault(b, 99));
        });

        if (!string.IsNullOrEmpty(key)) mods.Add(key);
        return string.Join("+", mods);
    }
}
