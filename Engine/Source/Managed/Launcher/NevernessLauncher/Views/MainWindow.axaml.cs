using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using NevernessLauncher.ViewModels;
using System.Linq;

namespace NevernessLauncher.Views
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();

            // 获取标题栏 Border 并添加拖动事件
            var titleBar = this.FindControl<Border>("TitleBar");
            if (titleBar != null)
            {
                titleBar.PointerPressed += OnTitleBarPointerPressed;
            }
        }

        private void OnTitleBarPointerPressed(object? sender, PointerPressedEventArgs e)
        {
            // 检查是否点击在按钮上
            var source = e.Source as Control;
            if (source is Button)
                return;

            // 开始窗口拖动
            if (e.GetCurrentPoint(this).Properties.IsLeftButtonPressed)
            {
                BeginMoveDrag(e);
            }
        }

        private void OnNavButtonClick(object? sender, RoutedEventArgs e)
        {
            if (sender is Button button && button.Tag is string tag)
            {
                var viewModel = DataContext as MainWindowViewModel;
                if (viewModel == null) return;

                var page = viewModel.Pages.FirstOrDefault(p => p.Title == tag);
                if (page != null)
                {
                    _ = viewModel.NavigateToPage(page);
                }
            }
        }

        private void OnMinimize(object? sender, RoutedEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }

        private void OnMaximize(object? sender, RoutedEventArgs e)
        {
            WindowState = WindowState == WindowState.Maximized
                ? WindowState.Normal
                : WindowState.Maximized;
        }

        private void OnClose(object? sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
