using Neverness.Editor.Assets.AssetFactories;
using Neverness.Editor.Script.Private;

namespace Neverness.Editor.Script.Public;

/// <summary>
/// 脚本编辑器模块入口。
/// </summary>
public static class ScriptEditorModule
{
    private static Private.ScriptEditorServiceImpl? _service;

    /// <summary>安装脚本编辑器模块。</summary>
    public static void Install(Neverness.Editor.Core.Public.IEditorContext context)
    {
        _service = new Private.ScriptEditorServiceImpl();
        context.RegisterService<IScriptEditorService>(_service);

        // 注册 C# 脚本资产工厂（ContentBrowser 右键菜单）
        AssetFactoryRegistry.Instance.Register(new CSharpScriptAssetFactory());

        // 初始化脚本资产索引（Phase A: GUID → FullName）
        ScriptAssetIndex.Instance.RebuildAll();
    }

    /// <summary>获取脚本编辑器服务。</summary>
    public static IScriptEditorService Service =>
        _service ?? throw new InvalidOperationException("ScriptEditorModule is not installed.");

    /// <summary>关闭脚本编辑器模块。</summary>
    public static void Shutdown()
    {
        _service?.Dispose();
        _service = null;
    }
}
