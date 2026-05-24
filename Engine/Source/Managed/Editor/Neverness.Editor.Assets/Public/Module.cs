using Neverness.Editor.Assets.AssetActions;
using Neverness.Editor.Assets.AssetFactories;
using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Scene;

namespace Neverness.Editor.Assets.Public;

/// <summary>
/// Neverness.Editor.Assets 模块安装入口。
/// Editor 启动时调用 <see cref="Install"/> 完成资产工厂系统的初始化。
/// </summary>
public static class AssetsModule
{
    /// <summary>安装资产工厂模块（自动发现 + 注册贡献者）。</summary>
    public static void Install(SceneManager sceneManager)
    {
        Private.AssetsModuleImp.Install(sceneManager);
    }

    /// <summary>每帧 Tick——批量保存脏缓存。</summary>
    public static void Tick()
    {
        Private.AssetsModuleImp.Tick();
    }
}
