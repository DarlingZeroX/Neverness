namespace Neverness.Editor.Framework.Public.Mvvm;

/// <summary>
/// Controller 基础接口——业务操作层。
/// 处理用户交互，修改 ViewModel 数据。
/// </summary>
public interface IController
{
    /// <summary>初始化控制器，绑定 ViewModel。</summary>
    void Initialize();

    /// <summary>清理控制器资源。</summary>
    void Shutdown();
}
