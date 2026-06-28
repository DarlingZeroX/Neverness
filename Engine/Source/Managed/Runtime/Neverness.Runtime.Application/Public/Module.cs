// Neverness.Runtime.Application — 模块入口。
// 暴露 SdlInputProvider 等公共服务，供 Editor/Script 层消费。

namespace Neverness.Runtime.Application;

/// <summary>
/// Neverness.Runtime.Application 模块入口。
///
/// 职责：
///   - 创建和管理 SdlInputProvider
///   - 提供全局访问点
///
/// 用法：
/// <code>
/// // EditorApplicationRunner 初始化时
/// ApplicationModule.Initialize();
///
/// // ScriptEditorModule.EnterPlayMode 时
/// var provider = ApplicationModule.InputProvider;
///
/// // 主循环每帧
/// ApplicationModule.NewFrame(deltaTime);
///
/// // 退出时
/// ApplicationModule.Shutdown();
/// </code>
/// </summary>
public static class ApplicationModule
{
    private static bool _initialized;

    /// <summary>模块是否已初始化。</summary>
    public static bool IsInitialized => _initialized;

    /// <summary>SDL 输入提供者（Initialize 后可用）。</summary>
    public static SdlInputProvider? InputProvider { get; private set; }

    /// <summary>
    /// 初始化应用模块——创建 SdlInputProvider。
    /// 应在 SDL 初始化后、Play Mode 之前调用。
    /// </summary>
    public static void Initialize()
    {
        if (_initialized)
        {
            Console.WriteLine("[ApplicationModule] 已初始化，跳过重复调用。");
            return;
        }

        InputProvider = new SdlInputProvider();
        _initialized = true;

        Console.WriteLine("[ApplicationModule] 已初始化，SdlInputProvider 就绪。");
    }

    /// <summary>
    /// 每帧开始时调用——清除输入瞬态标志（Down/Up）。
    /// 必须在 Gameplay Tick 之前调用。
    /// </summary>
    /// <param name="deltaTime">帧间隔时间。</param>
    public static void NewFrame(float deltaTime)
    {

    }

    public static void EndFrame(float deltaTime)
    {
        InputProvider?.NewFrame(deltaTime);
    }

    /// <summary>
    /// 关闭应用模块——清理资源。
    /// 应在应用退出时调用。
    /// </summary>
    public static void Shutdown()
    {
        if (!_initialized)
            return;

        InputProvider = null;
        _initialized = false;

        Console.WriteLine("[ApplicationModule] 已关闭。");
    }
}
