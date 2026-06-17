using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 纹理查看器浮动窗口——独立窗口，可拖拽停靠到主窗口。
/// </summary>
public class TextureViewerFloatingWindow : Window
{
    private readonly GUID _assetGuid;

    public TextureViewerFloatingWindow(string assetName, GUID assetGuid, TextureViewerControl viewerControl)
    {
        _assetGuid = assetGuid;

        // 窗口设置
        Title = $"Texture - {assetName}";
        Width = 600;
        Height = 500;
        MinWidth = 200;
        MinHeight = 200;
        WindowStartupLocation = WindowStartupLocation.CenterScreen;
        Background = new SolidColorBrush(Color.Parse("#FF252526"));

        // 设置内容
        Content = viewerControl;

        // 窗口关闭时的处理
        Closed += OnWindowClosed;
    }

    private void OnWindowClosed(object? sender, EventArgs e)
    {
        // TODO: 取消注册 AssetEditorManager 映射
        Console.WriteLine($"[TextureViewerFloatingWindow] 窗口关闭: {_assetGuid.ToHexString()}");
    }
}
