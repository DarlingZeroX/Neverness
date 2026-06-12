using Avalonia.Controls;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// Avalonia 组件检查器注册表——自动发现和管理 Inspector。
///
/// 与 ImGui 的 ComponentInspectorRegistry 对应。
/// 使用程序集扫描自动发现所有 AvaloniaInspectorBase 实现。
/// </summary>
public static class AvaloniaComponentInspectorRegistry
{
    private static readonly List<AvaloniaInspectorBase> _inspectors = new();
    private static bool _discovered;

    /// <summary>已注册的检查器列表。</summary>
    public static IReadOnlyList<AvaloniaInspectorBase> Inspectors => _inspectors;

    /// <summary>
    /// 从指定程序集发现检查器。
    /// </summary>
    public static void DiscoverFromAssembly(System.Reflection.Assembly assembly)
    {
        if (_discovered) return;

        var inspectorTypes = assembly.GetTypes()
            .Where(t => !t.IsAbstract && typeof(AvaloniaInspectorBase).IsAssignableFrom(t));

        foreach (var type in inspectorTypes)
        {
            try
            {
                var inspector = (AvaloniaInspectorBase)Activator.CreateInstance(type)!;
                _inspectors.Add(inspector);
                Console.WriteLine($"[AvaloniaComponentInspectorRegistry] 发现 Inspector: {inspector.DisplayName} ({type.Name})");
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"[AvaloniaComponentInspectorRegistry] 创建 Inspector 失败: {type.Name} - {ex.Message}");
            }
        }

        _discovered = true;
        Console.WriteLine($"[AvaloniaComponentInspectorRegistry] 共发现 {_inspectors.Count} 个 Inspector");
    }

    /// <summary>
    /// 手动注册检查器。
    /// </summary>
    public static void Register(AvaloniaInspectorBase inspector)
    {
        if (!_inspectors.Contains(inspector))
        {
            _inspectors.Add(inspector);
        }
    }

    /// <summary>
    /// 获取指定组件类型的检查器。
    /// </summary>
    public static AvaloniaInspectorBase? GetInspector(ulong typeId)
    {
        return _inspectors.FirstOrDefault(i => i.CanInspect(typeId));
    }

    /// <summary>
    /// 创建指定组件类型的检查器 UI。
    /// </summary>
    public static Control? CreateInspectorUI(ulong sceneHandle, ulong entityHandle, ulong typeId)
    {
        var inspector = GetInspector(typeId);
        if (inspector == null)
        {
            // 没有找到专用检查器，显示通用信息
            return CreateGenericInspector(typeId);
        }

        return inspector.CreateInspector(sceneHandle, entityHandle, typeId);
    }

    /// <summary>
    /// 创建通用检查器（当没有专用检查器时使用）。
    /// </summary>
    private static Control CreateGenericInspector(ulong typeId)
    {
        var panel = new StackPanel
        {
            Spacing = 4,
            Margin = new Avalonia.Thickness(4),
        };

        var infoText = new TextBlock
        {
            Text = $"No inspector for component type: {typeId}",
            FontSize = 12,
            Foreground = new Avalonia.Media.SolidColorBrush(Avalonia.Media.Color.Parse("#FF999999")),
        };
        panel.Children.Add(infoText);

        return panel;
    }

    /// <summary>
    /// 清空注册表。
    /// </summary>
    public static void Clear()
    {
        _inspectors.Clear();
        _discovered = false;
    }
}
