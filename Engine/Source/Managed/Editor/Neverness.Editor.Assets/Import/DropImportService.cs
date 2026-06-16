using System.Reflection;
using Neverness.Editor.Core.Public;

namespace Neverness.Editor.Assets.Private;

/// <summary>
/// 拖放导入服务——监听 EditorEventBus 的 AssetCreated 事件，
/// 将外部拖入的文件路由到对应的 <see cref="IDropFileHandler"/>。
/// </summary>
internal sealed class DropImportService
{
    private readonly List<IDropFileHandler> _handlers = new();
    private readonly IEditorEventBus _eventBus;
    private readonly string _assetsRoot;

    public DropImportService(IEditorEventBus eventBus, string assetsRoot)
    {
        _eventBus = eventBus;
        _assetsRoot = Path.GetFullPath(assetsRoot);
    }

    public void Start()
    {
        DiscoverHandlers();
        _eventBus.Subscribe(EditorEventType.AssetCreated, HandleAssetCreated);
    }

    public void Stop()
    {
        _eventBus.Unsubscribe(EditorEventType.AssetCreated, HandleAssetCreated);
    }

    /// <summary>手动注册处理器（测试或插件用），优先级最高。</summary>
    public void RegisterHandler(IDropFileHandler handler)
    {
        _handlers.Insert(0, handler);
    }

    private void HandleAssetCreated(EditorEvent evt)
    {
        if (evt.Payload is not string filePath)
            return;

        // 只处理外部文件（Assets 目录外的）
        if (IsInsideAssets(filePath))
            return;

        if (!File.Exists(filePath))
            return;

        foreach (var handler in _handlers)
        {
            if (!handler.CanHandle(filePath))
                continue;

            try
            {
                handler.Handle(filePath, _assetsRoot);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"[DropImportService] 处理器失败: {filePath} → {ex.Message}");
            }
            return;
        }
    }

    private bool IsInsideAssets(string filePath)
    {
        var normalized = Path.GetFullPath(filePath);
        return normalized.StartsWith(_assetsRoot, StringComparison.OrdinalIgnoreCase);
    }

    private void DiscoverHandlers()
    {
        foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
        {
            var name = assembly.GetName().Name;
            if (name == null || !name.StartsWith("Neverness.Editor"))
                continue;

            Type[] types;
            try { types = assembly.GetTypes(); }
            catch { continue; }

            foreach (var type in types)
            {
                if (type.IsAbstract || type.IsInterface)
                    continue;
                if (!typeof(IDropFileHandler).IsAssignableFrom(type))
                    continue;

                var attr = type.GetCustomAttribute<DropFileHandlerAttribute>();
                if (attr == null)
                    continue;

                // 跳过无扩展名的兜底处理器（由末尾显式添加）
                if (attr.Extensions.Length == 0)
                    continue;

                if (Activator.CreateInstance(type) is IDropFileHandler handler)
                    _handlers.Add(handler);
            }
        }

        // 兜底处理器放最后
        _handlers.Add(new DefaultDropHandler());
    }
}
