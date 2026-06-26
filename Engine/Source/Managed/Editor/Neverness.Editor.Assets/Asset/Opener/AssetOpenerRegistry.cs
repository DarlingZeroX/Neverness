using System.Reflection;
using Neverness.Editor.Core.Public;

namespace Neverness.Editor.Assets.AssetOpening;

/// <summary>
/// 资产打开器注册表——反射自动发现所有带 <see cref="AssetOpenerAttribute"/> 的 <see cref="IAssetOpener"/> 实现。
///
/// 发现策略（与 <see cref="ImporterRegistry"/> 一致）：
///   - 扫描当前 AppDomain 中所有 Neverness.Editor.* 程序集
///   - 找到实现 IAssetOpener 且标记 [AssetOpener] 的非抽象类
///   - 构造函数参数从 <see cref="IEditorContext"/> 服务容器解析
///   - 按 TypeId 注册，后注册覆盖先注册
/// </summary>
public sealed class AssetOpenerRegistry
{
    private readonly Dictionary<ulong, IAssetOpener> _openers = new();
    private readonly List<IAssetOpener> _allOpeners = new();

    /// <summary>所有已注册的打开器。</summary>
    public IReadOnlyList<IAssetOpener> All => _allOpeners;

    /// <summary>
    /// 自动发现所有 opener。构造函数参数从 <paramref name="context"/> 解析。
    /// </summary>
    public void Discover(IEditorContext context)
    {
        _openers.Clear();
        _allOpeners.Clear();

        Console.WriteLine($"[AssetOpenerRegistry] 开始发现 opener...");

        foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
        {
            if (!IsTargetAssembly(assembly))
                continue;

            Type[] types;
            try { types = assembly.GetTypes(); }
            catch (Exception ex)
            {
                Console.WriteLine($"[AssetOpenerRegistry] 扫描程序集失败: {assembly.GetName().Name} → {ex.Message}");
                continue;
            }

            foreach (var type in types)
            {
                if (type.IsAbstract || type.IsInterface)
                    continue;

                if (!typeof(IAssetOpener).IsAssignableFrom(type))
                    continue;

                var attr = type.GetCustomAttribute<AssetOpenerAttribute>();
                if (attr == null)
                    continue;

                Console.WriteLine($"[AssetOpenerRegistry] 发现 opener: {type.FullName} (TypeId={attr.TypeId})");

                var opener = TryCreateOpener(type, context);
                if (opener == null)
                {
                    Console.WriteLine($"[AssetOpenerRegistry] 无法实例化 opener: {type.FullName}（缺少依赖）");
                    continue;
                }

                _allOpeners.Add(opener);
                _openers[attr.TypeId] = opener;
                Console.WriteLine($"[AssetOpenerRegistry] 注册 opener: {type.FullName} (TypeId={attr.TypeId})");
            }
        }

        Console.WriteLine($"[AssetOpenerRegistry] 发现 {_allOpeners.Count} 个 opener");
    }

    /// <summary>按 TypeId 获取对应的 opener。未找到返回 null。</summary>
    public IAssetOpener? GetOpener(ulong typeId)
    {
        _openers.TryGetValue(typeId, out var opener);
        return opener;
    }

    /// <summary>检查是否有 opener 支持该 TypeId。</summary>
    public bool IsSupported(ulong typeId) => _openers.ContainsKey(typeId);

    /// <summary>手动注册 opener（用于延迟加载的程序集）。</summary>
    public void Register(IAssetOpener opener)
    {
        // 从 AssetOpenerAttribute 获取 TypeId
        var attr = opener.GetType().GetCustomAttribute<AssetOpenerAttribute>();
        if (attr == null)
        {
            Console.WriteLine($"[AssetOpenerRegistry] 无法注册 opener: {opener.GetType().FullName}（缺少 AssetOpenerAttribute）");
            return;
        }

        _allOpeners.Add(opener);
        _openers[attr.TypeId] = opener;
        Console.WriteLine($"[AssetOpenerRegistry] 手动注册 opener: {opener.GetType().FullName} (TypeId={attr.TypeId})");
    }

    /// <summary>
    /// 尝试实例化 opener：优先无参构造，其次从 context 解析构造函数参数。
    /// </summary>
    private static IAssetOpener? TryCreateOpener(Type type, IEditorContext context)
    {
        // 优先无参构造
        var parameterless = type.GetConstructor(Type.EmptyTypes);
        if (parameterless != null)
        {
            try { return parameterless.Invoke(null) as IAssetOpener; }
            catch { return null; }
        }

        // 选择参数最多的构造（最具体的）
        var ctor = type.GetConstructors()
            .OrderByDescending(c => c.GetParameters().Length)
            .FirstOrDefault();

        if (ctor == null)
            return null;

        var parameters = ctor.GetParameters();
        var args = new object[parameters.Length];

        for (int i = 0; i < parameters.Length; i++)
        {
            var method = typeof(IEditorContext).GetMethod(nameof(IEditorContext.TryGetService))!
                .MakeGenericMethod(parameters[i].ParameterType);
            var args_array = new object?[] { null };
            var found = (bool)method.Invoke(context, args_array)!;
            if (!found)
                return null;
            args[i] = args_array[0]!;
        }

        try { return ctor.Invoke(args) as IAssetOpener; }
        catch { return null; }
    }

    private static bool IsTargetAssembly(Assembly assembly)
    {
        var name = assembly.GetName().Name;
        return name != null
            && (name.StartsWith("Neverness.Editor", StringComparison.OrdinalIgnoreCase)
                || name.StartsWith("Neverness.Runtime", StringComparison.OrdinalIgnoreCase));
    }
}
