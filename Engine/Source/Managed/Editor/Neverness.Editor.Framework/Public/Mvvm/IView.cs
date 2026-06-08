namespace Neverness.Editor.Framework.Public.Mvvm;

/// <summary>
/// View 基础接口——UI 渲染层。
/// ImGuiFrontend 和 AvaloniaFrontend 各自实现此接口。
/// </summary>
public interface IView
{
    /// <summary>关联的 ViewModel 类型。</summary>
    Type ViewModelType { get; }

    /// <summary>绑定 ViewModel。</summary>
    void Bind(object viewModel);

    /// <summary>解绑 ViewModel。</summary>
    void Unbind();

    /// <summary>渲染 UI（每帧调用）。</summary>
    void Render();
}
