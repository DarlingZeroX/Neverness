using System.Reflection;

namespace Neverness.Editor.Assets;

/// <summary>
/// Importer 注册表。
///
/// 通过反射自动发现所有标记 [AssetImporter] 的 IAssetImporter 实现，
/// 提供按扩展名查询 importer 的能力。
///
/// 发现策略：
///   - 扫描当前 AppDomain 中所有 Neverness.Editor.* 程序集
///   - 找到实现 IAssetImporter 且标记 [AssetImporter] 的非抽象类
///   - 支持优先级排序（同扩展名取 Priority 最高的）
/// </summary>
public static class ImporterRegistry
{
    private static readonly Dictionary<string, IAssetImporter> s_extensionToImporter
        = new(StringComparer.OrdinalIgnoreCase);

    private static readonly Dictionary<string, IAssetImporter> s_nameToImporter
        = new(StringComparer.OrdinalIgnoreCase);

    private static readonly List<IAssetImporter> s_allImporters = new();

    private static bool s_discovered;

    private static readonly object s_lock = new();

    /// <summary>是否已完成发现。</summary>
    public static bool IsDiscovered
    {
        get { lock (s_lock) return s_discovered; }
    }

    /// <summary>所有已发现的 importer（只读）。</summary>
    public static IReadOnlyList<IAssetImporter> All
    {
        get { lock (s_lock) return s_allImporters.ToList(); }
    }

    /// <summary>
    /// 自动发现所有 importer。
    /// 应在编辑器启动时调用一次。
    /// </summary>
    public static void Discover()
    {
        lock (s_lock)
        {
            if (s_discovered)
                return;

            s_extensionToImporter.Clear();
            s_nameToImporter.Clear();
            s_allImporters.Clear();

            var importerTypes = new List<(Type type, AssetImporterAttribute attr)>();

            /* 扫描所有相关程序集 */
            foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                if (!IsTargetAssembly(assembly))
                    continue;

                Type[] types;
                try { types = assembly.GetTypes(); }
                catch { continue; }

                foreach (var type in types)
                {
                    if (type.IsAbstract || type.IsInterface)
                        continue;

                    if (!typeof(IAssetImporter).IsAssignableFrom(type))
                        continue;

                    var attr = type.GetCustomAttribute<AssetImporterAttribute>();
                    if (attr == null)
                        continue;

                    importerTypes.Add((type, attr));
                }
            }

            /* 按优先级降序排列 */
            importerTypes.Sort((a, b) => b.attr.Priority.CompareTo(a.attr.Priority));

            /* 实例化并注册 */
            foreach (var (type, attr) in importerTypes)
            {
                IAssetImporter? importer;
                try
                {
                    importer = Activator.CreateInstance(type) as IAssetImporter;
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"[ImporterRegistry] 实例化 importer 失败: {type.FullName} → {ex.Message}");
                    continue;
                }

                if (importer == null)
                    continue;

                s_allImporters.Add(importer);

                /* 按类型名称注册（用于 meta 文件中的 importer 名称查找） */
                var typeName = type.Name;
                if (!s_nameToImporter.ContainsKey(typeName))
                    s_nameToImporter[typeName] = importer;

                foreach (var ext in attr.Extensions)
                {
                    if (!s_extensionToImporter.ContainsKey(ext))
                        s_extensionToImporter[ext] = importer;
                }
            }

            s_discovered = true;
            Console.WriteLine($"[ImporterRegistry] 发现 {s_allImporters.Count} 个 importer，覆盖 {s_extensionToImporter.Count} 种扩展名");
        }
    }

    /// <summary>按扩展名获取对应的 importer。未找到返回 null。</summary>
    public static IAssetImporter? GetImporter(string extension)
    {
        lock (s_lock)
        {
            EnsureDiscovered();

            var ext = extension.ToLowerInvariant();
            if (!ext.StartsWith('.'))
                ext = '.' + ext;

            return s_extensionToImporter.TryGetValue(ext, out var importer) ? importer : null;
        }
    }

    /// <summary>按类型名称获取对应的 importer（如 "TextureImporter"）。未找到返回 null。</summary>
    public static IAssetImporter? GetImporterByName(string name)
    {
        lock (s_lock)
        {
            EnsureDiscovered();
            return s_nameToImporter.TryGetValue(name, out var importer) ? importer : null;
        }
    }

    /// <summary>按文件路径获取 importer（根据扩展名推断）。</summary>
    public static IAssetImporter? GetImporterForFile(string filePath)
    {
        return GetImporter(Path.GetExtension(filePath));
    }

    /// <summary>检查是否有 importer 支持该扩展名。</summary>
    public static bool IsSupported(string extension)
    {
        lock (s_lock)
        {
            EnsureDiscovered();

            var ext = extension.ToLowerInvariant();
            if (!ext.StartsWith('.'))
                ext = '.' + ext;

            return s_extensionToImporter.ContainsKey(ext);
        }
    }

    /// <summary>获取所有支持的扩展名列表。</summary>
    public static IReadOnlyList<string> SupportedExtensions
    {
        get
        {
            lock (s_lock)
            {
                EnsureDiscovered();
                return s_extensionToImporter.Keys.ToList();
            }
        }
    }

    /// <summary>强制重新发现（测试用）。</summary>
    public static void Rediscover()
    {
        lock (s_lock)
        {
            s_discovered = false;
            Discover();
        }
    }

    private static void EnsureDiscovered()
    {
        if (!s_discovered)
            Discover();
    }

    private static bool IsTargetAssembly(Assembly assembly)
    {
        var name = assembly.GetName().Name;
        return name != null
            && (name.StartsWith("Neverness.Editor", StringComparison.OrdinalIgnoreCase)
                || name.StartsWith("Neverness.Runtime", StringComparison.OrdinalIgnoreCase));
    }
}
