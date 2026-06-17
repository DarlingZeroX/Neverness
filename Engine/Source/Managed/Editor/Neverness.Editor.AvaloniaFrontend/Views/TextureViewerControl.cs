using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Neverness.Editor.Assets.AssetOpening;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.AvaloniaFrontend.Views;

/// <summary>
/// 纹理查看器控件——显示纹理资产的预览。
/// 支持平移，集成到 Dock 面板。
/// </summary>
public class TextureViewerControl : UserControl
{
    private readonly GUID _assetGuid;
    private readonly AssetEditorManager _editorManager;
    private Image? _imageControl;
    private TextBlock? _infoText;

    public TextureViewerControl(string assetName, GUID assetGuid, AssetEditorManager editorManager)
    {
        _assetGuid = assetGuid;
        _editorManager = editorManager;

        // 创建 UI
        CreateUI();
    }

    private void CreateUI()
    {
        var root = new DockPanel();

        // 顶部信息栏
        var infoBar = CreateInfoBar();
        DockPanel.SetDock(infoBar, Avalonia.Controls.Dock.Top);
        root.Children.Add(infoBar);

        // 图片显示区域（支持平移）
        var scrollViewer = new ScrollViewer
        {
            Background = new SolidColorBrush(Color.Parse("#FF1E1E1E")),
            HorizontalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Auto,
            VerticalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Auto,
        };

        _imageControl = new Image
        {
            Stretch = Stretch.Uniform,  // 自动缩放以适应窗口
            HorizontalAlignment = HorizontalAlignment.Center,
            VerticalAlignment = VerticalAlignment.Center,
        };

        scrollViewer.Content = _imageControl;
        root.Children.Add(scrollViewer);

        Content = root;
    }

    private Control CreateInfoBar()
    {
        var infoBar = new DockPanel
        {
            Background = new SolidColorBrush(Color.Parse("#FF2D2D30")),
            Height = 32,
            Margin = new Thickness(0),
        };

        _infoText = new TextBlock
        {
            Text = "Loading...",
            FontSize = 12,
            Foreground = new SolidColorBrush(Color.Parse("#FFCCCCCC")),
            VerticalAlignment = VerticalAlignment.Center,
            Margin = new Thickness(8, 0),
        };
        infoBar.Children.Add(_infoText);

        return infoBar;
    }

    /// <summary>更新信息文本。</summary>
    private void UpdateInfoText(int width, int height)
    {
        if (_infoText == null)
            return;

        _infoText.Text = $"{width} x {height} | RGBA8 | {_assetGuid.ToHexString()}";
    }

    /// <summary>设置纹理数据（RGBA8 格式）。</summary>
    public void SetTextureData(ReadOnlySpan<byte> rgbaData, int width, int height)
    {
        if (_imageControl == null || rgbaData.IsEmpty || width <= 0 || height <= 0)
            return;

        try
        {
            // 创建 WriteableBitmap
            var bitmap = new WriteableBitmap(
                new PixelSize(width, height),
                new Vector(96, 96),
                Avalonia.Platform.PixelFormat.Rgba8888,
                Avalonia.Platform.AlphaFormat.Premul);

            // 写入像素数据
            using (var fb = bitmap.Lock())
            {
                var stride = fb.RowBytes;
                var dest = fb.Address;

                for (int y = 0; y < height; y++)
                {
                    var srcRow = rgbaData.Slice(y * width * 4, width * 4);
                    var destRow = dest + y * stride;
                    System.Runtime.InteropServices.Marshal.Copy(srcRow.ToArray(), 0, destRow, width * 4);
                }
            }

            _imageControl.Source = bitmap;

            // 更新信息
            UpdateInfoText(width, height);
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[TextureViewerControl] 设置纹理数据失败: {ex.Message}");
        }
    }

    /// <summary>从 AssetHandle 加载并显示纹理。</summary>
    public void LoadFromAssetHandle(AssetHandle handle)
    {
        if (handle.IsZero)
        {
            if (_infoText != null)
                _infoText.Text = "Invalid asset handle";
            return;
        }

        try
        {
            // 获取纹理类型信息
            var typeInfoBlob = AssetManager.Instance.GetBlobByType(handle.Value, Neverness.Editor.Assets.AssetTypeId.BlobType.TypeInfo);
            if (typeInfoBlob.IsEmpty)
            {
                Console.Error.WriteLine("[TextureViewerControl] 无法获取纹理类型信息");
                if (_infoText != null)
                    _infoText.Text = "Missing type info";
                return;
            }

            // 解析类型信息（24 字节：width:u32, height:u32, format:u32, mipCount:u32, arraySize:u32, flags:u32）
            if (typeInfoBlob.Length < 24)
            {
                Console.Error.WriteLine("[TextureViewerControl] 类型信息数据太短");
                return;
            }

            var width = BitConverter.ToInt32(typeInfoBlob.Slice(0, 4));
            var height = BitConverter.ToInt32(typeInfoBlob.Slice(4, 8));
            var format = BitConverter.ToInt32(typeInfoBlob.Slice(8, 12));

            // 获取 mip0 数据（BlobType.Data = 0）
            var pixelData = AssetManager.Instance.GetBlobByType(handle.Value, Neverness.Editor.Assets.AssetTypeId.BlobType.Data);
            if (pixelData.IsEmpty)
            {
                Console.Error.WriteLine("[TextureViewerControl] 无法获取纹理像素数据");
                if (_infoText != null)
                    _infoText.Text = "Missing pixel data";
                return;
            }

            // 设置纹理数据
            SetTextureData(pixelData, width, height);
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[TextureViewerControl] 加载纹理失败: {ex.Message}");
            if (_infoText != null)
                _infoText.Text = $"Load failed: {ex.Message}";
        }
    }
}
