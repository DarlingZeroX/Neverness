using Hexa.NET.ImGui;
using Neverness.Editor.Assets.Private.Context;
using Neverness.Editor.Core.Public;
using Neverness.Editor.Core.ViewModels;
using Neverness.Editor.Core.Controllers;
using Neverness.Editor.Framework.Public;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public.Mvvm;
using Neverness.Runtime.Assets;
using System.Numerics;

namespace Neverness.Editor.ImGuiFrontend.Views;

/// <summary>
/// 内容浏览器 ImGui View——读取 ContentBrowserViewModel，渲染 ImGui 内容浏览器 UI。
///
/// 从 ContentBrowserPanel 迁移，保留原有 UI 风格和交互逻辑。
/// </summary>
public class ContentBrowserImGuiView : PanelViewBase
{
    private ContentBrowserViewModel? _viewModel;
    private ContentBrowserController? _controller;

    // ── 缩略图绘制器（从 ContentBrowserFileUIBox 迁移）──
    private readonly ThumbnailRenderer _thumbnail;

    // ── 目录树展开状态 ──
    private readonly HashSet<string> _expandedDirectories = new();

    public ContentBrowserImGuiView()
        : base("Content Browser", "🖥️ Content Browser")
    {
        _thumbnail = new ThumbnailRenderer();
    }

    /// <summary>设置 Controller（由 CompositionRoot 调用）。</summary>
    public void SetController(ContentBrowserController controller)
    {
        _controller = controller;
    }

    public override Type ViewModelType => typeof(ContentBrowserViewModel);

    public override void Bind(object viewModel)
    {
        _viewModel = (ContentBrowserViewModel)viewModel;
        _viewModel.PropertyChanged += OnPropertyChanged;
    }

    public override void Unbind()
    {
        if (_viewModel != null)
            _viewModel.PropertyChanged -= OnPropertyChanged;
        _viewModel = null;
    }

    public override void Render()
    {
        if (_viewModel == null || _controller == null) return;

        if (!ImGui.Begin(GetWindowFullName(),
            ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.MenuBar))
        {
            ImGui.End();
            return;
        }

        // 菜单栏
        if (ImGui.BeginMenuBar())
        {
            DrawBrowserHeader();
            ImGui.EndMenuBar();
        }

        // 左侧目录树
        ImGui.BeginChild("Content Browser Left",
            new Vector2(200, 0),
            ImGuiChildFlags.Borders | ImGuiChildFlags.ResizeX);
        DrawRootDirectoryTree();
        ImGui.EndChild();

        ImGui.SameLine();

        // 右侧内容浏览器
        ImGui.BeginChild("Content Browser Right",
            Vector2.Zero,
            ImGuiChildFlags.Borders);
        DrawContentBrowser();
        ImGui.EndChild();

        ImGui.End();
    }

    // ── 导航栏 ──
    private void DrawBrowserHeader()
    {
        using (new ImGuiEx.StyleColor(ImGuiCol.Button, new Vector4(0, 0, 0, 0)))
        {
            Vector2 thumbnailSize = new Vector2(25.0f, 24.0f);

            // 后退按钮
            if (_viewModel!.CanGoBack)
            {
                if (ImGui.Button("◀" + "##Back", thumbnailSize))
                {
                    _controller?.GoBack();
                }
            }
            else
            {
                ImGui.BeginDisabled();
                ImGui.Button("◀" + "##Back", thumbnailSize);
                ImGui.EndDisabled();
            }

            ImGui.SameLine();

            // 刷新按钮
            if (ImGui.Button("🔄" + "##Refresh", thumbnailSize))
            {
                _controller?.RefreshDirectory();
            }

            ImGui.SameLine();

            // 新建文件夹按钮
            if (ImGui.Button("📁" + "##NewFolder", thumbnailSize))
            {
                _controller?.CreateNewFolder();
            }

            // 绘制路径栏
            DrawBreadcrumb();
        }
    }

    private void DrawBreadcrumb()
    {
        var currentDir = _viewModel!.CurrentDirectory;
        var assetDir = _viewModel.AssetDirectory;

        if (string.IsNullOrEmpty(currentDir) || string.IsNullOrEmpty(assetDir))
            return;

        string relativePath = Path.GetRelativePath(assetDir, currentDir);

        ImGui.Separator();

        string[] splitItems = relativePath.Split(
            Path.DirectorySeparatorChar,
            StringSplitOptions.RemoveEmptyEntries);

        int pathPos = 0;

        foreach (var item in splitItems)
        {
            pathPos += item.Length + 1;

            if (item == ".")
                continue;

            if (ImGui.Button(item, Vector2.Zero))
            {
                string subPath = relativePath[..Math.Min(pathPos - 1, relativePath.Length)];
                string targetPath = Path.Combine(assetDir, subPath);
                _controller?.OpenDirectory(targetPath);
                break;
            }

            ImGui.SameLine();
            ImGui.Text("›");
            ImGui.SameLine();
        }
    }

    // ── 目录树（左侧）──
    private void DrawRootDirectoryTree()
    {
        if (_controller == null) return;

        try
        {
            // 获取目录树根节点
            var rootDir = _controller.GetDirectoryTreeRoot();
            if (rootDir == null)
            {
                ImGui.TextDisabled("No directory tree");
                return;
            }

            // 渲染根目录
            if (ImGui.CollapsingHeader("Assets##ContentBrowserPanel", ImGuiTreeNodeFlags.DefaultOpen))
            {
                // 点击打开根目录
                if (ImGui.IsItemClicked())
                {
                    _controller.OpenDirectory(rootDir.SystemPath.FullPath);
                }

                // 遍历并绘制子目录
                foreach (var child in rootDir.Directories)
                {
                    DrawDirectoryTree(child);
                }
            }
        }
        catch (Exception ex)
        {
            ImGui.TextDisabled($"Error: {ex.Message}");
        }
    }

    private void DrawDirectoryTree(ContentDirectoryNode node)
    {
        ImGui.PushID(node.SystemPath.FullPath);

        string nodeName = $"📁 {node.Name}";

        // TreeNodeEx 绘制
        if (ImGui.TreeNode(nodeName))
        {
            foreach (var child in node.Directories)
            {
                DrawDirectoryTree(child);
            }
            ImGui.TreePop();
        }

        // 点击打开目录
        if (ImGui.IsItemHovered() && ImGui.IsMouseClicked(ImGuiMouseButton.Left))
        {
            _controller?.OpenDirectory(node.SystemPath.FullPath);
        }

        ImGui.PopID();
    }

    // ── 内容区域（右侧）──
    private void DrawContentBrowser()
    {
        if (_controller == null) return;

        try
        {
            // 使用旧的 UI 风格绘制
            using (new ImGuiEx.StyleColor(ImGuiCol.Button, new Vector4(0, 0, 0, 0)))
            using (new ImGuiEx.StyleVar(ImGuiStyleVar.ItemSpacing, new Vector2(4, 8)))
            {
                Vector2 windowPos = ImGui.GetWindowPos();
                Vector2 windowSize = ImGui.GetWindowSize();
                Vector2 windowEndPos = new Vector2(windowPos.X + windowSize.X, windowPos.Y + windowSize.Y);

                // 计算列数
                float panelWidth = ImGui.GetContentRegionAvail().X;
                int columnCount = (int)(panelWidth / _thumbnail.CellSize);
                if (columnCount < 1) columnCount = 1;

                ImGui.Columns(columnCount, "##ContentColumns", false);
                ImDrawListPtr drawList = ImGui.GetWindowDrawList();

                // 获取目录内容
                var subdirectories = _controller.GetSubdirectories();
                var files = _controller.GetFiles();

                // 绘制目录
                foreach (var dir in subdirectories)
                {
                    DrawGridItem(drawList, dir.Name, dir.SystemPath.FullPath, true, windowPos, windowEndPos);
                    if (ImGui.IsItemHovered() && ImGui.IsMouseDoubleClicked(ImGuiMouseButton.Left))
                    {
                        _controller.OpenDirectory(dir.SystemPath.FullPath);
                    }
                    ShowItemContextMenu(dir.SystemPath.FullPath, dir.Name, true);
                    ImGui.NextColumn();
                }

                // 绘制文件
                foreach (var file in files)
                {
                    DrawGridItem(drawList, file.Name, file.SystemPath.FullPath, false, windowPos, windowEndPos, file.AssetType);
                    if (ImGui.IsItemHovered() && ImGui.IsMouseDoubleClicked(ImGuiMouseButton.Left))
                    {
                        _controller.OpenFile(file.SystemPath.FullPath);
                    }

                    // 资产拖拽源
                    if (!file.AssetGuid.IsZero)
                    {
                        AssetDragDrop.SetDragDropPayload(file.AssetGuid, file.AssetType, file.Name);
                    }

                    ShowItemContextMenu(file.SystemPath.FullPath, file.Name, false);
                    ImGui.NextColumn();
                }

                ImGui.Columns(1);

                // 背景右键菜单（空白区域）
                ShowBackgroundContextMenu();
            }
        }
        catch (Exception ex)
        {
            ImGui.TextDisabled($"Error: {ex.Message}");
        }
    }

    private void DrawGridItem(ImDrawListPtr drawList, string name, string path, bool isDir,
        Vector2 windowPos, Vector2 windowEndPos, string assetType = "")
    {
        ImGui.PushID(path);

        // 不可见按钮作为交互区域
        ImGui.InvisibleButton(path, isDir ? _thumbnail.ImageSize : _thumbnail.Size);

        Vector2 p0 = ImGui.GetItemRectMin();
        Vector2 p1 = new Vector2(p0.X + _thumbnail.Size.X, p0.Y + _thumbnail.Size.Y);

        // 裁剪检查
        if (p0.Y > windowEndPos.Y || p1.Y < windowPos.Y)
        {
            ImGui.PopID();
            return;
        }

        // 绘制缩略图
        _thumbnail.Draw(drawList, name, path, p0, p1, isDir, assetType);

        ImGui.PopID();
    }

    private void ShowItemContextMenu(string path, string name, bool isDirectory)
    {
        if (ImGui.BeginPopupContextItem())
        {
            if (ImGui.MenuItem("Open"))
            {
                if (isDirectory)
                    _controller?.OpenDirectory(path);
                else
                    _controller?.OpenFile(path);
            }

            ImGui.Separator();

            if (ImGui.MenuItem("Rename"))
            {
                // TODO: 弹出重命名对话框
            }

            if (ImGui.MenuItem("Delete"))
            {
                _controller?.DeleteItem(path);
            }

            ImGui.Separator();

            if (ImGui.MenuItem("Copy Path"))
            {
                // TODO: 复制路径到剪贴板
            }

            if (isDirectory && ImGui.MenuItem("New Folder"))
            {
                _controller?.CreateNewFolder();
            }

            ImGui.EndPopup();
        }
    }

    /// <summary>背景右键菜单（空白区域）。</summary>
    private void ShowBackgroundContextMenu()
    {
        var ctx = ContextMenuManager.Instance;

        // 注入运行时上下文
        ctx.SetContext(ContentBrowserContextMenu.KeyPath, _viewModel?.CurrentDirectory ?? "");

        // 渲染菜单
        ctx.RenderWindowContextMenu(ContentBrowserContextMenu.BackgroundId);
    }

    private void OnPropertyChanged(string propertyName)
    {
        // ImGui 即时模式，每帧重新读取 ViewModel
    }

    // ── 缩略图绘制器（从 ContentBrowserFileUIBox 迁移）──
    private class ThumbnailRenderer
    {
        public float ThumbnailTextSizeY { get; } = 35.0f;
        public float Padding { get; } = 6.0f;
        public float ThumbnailImageSizeY { get; } = 90.0f;

        public float CellSize { get; }
        public Vector2 Size { get; }
        public Vector2 TextSize { get; }
        public Vector2 ImageSize { get; }
        public uint AssetTypeColor { get; }

        public ThumbnailRenderer()
        {
            Size = new Vector2(ThumbnailImageSizeY, ThumbnailImageSizeY + ThumbnailTextSizeY);
            CellSize = Size.X + Padding;
            TextSize = new Vector2(CellSize, ThumbnailTextSizeY);
            ImageSize = new Vector2(Size.X, Size.Y - ThumbnailTextSizeY);
            AssetTypeColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.39f, 0.39f, 0.39f, 1.0f));
        }

        public void Draw(ImDrawListPtr drawList, string name, string path,
            Vector2 p0, Vector2 p1, bool isDir = true, string assetType = "")
        {
            Vector2 textPosStart = new Vector2(p0.X, p0.Y + Size.Y - ThumbnailTextSizeY);
            Vector2 assetTypeStart = new Vector2(p0.X, p0.Y + Size.Y - 18);
            Vector2 textPosEnd = new Vector2(textPosStart.X + TextSize.X, textPosStart.Y + TextSize.Y);

            Vector2 imagePosStart = new Vector2(p0.X, p0.Y);
            Vector2 imagePosEnd = new Vector2(imagePosStart.X + ImageSize.X, imagePosStart.Y + ImageSize.Y);
            Vector2 thumbnailPosEnd = new Vector2(imagePosStart.X + Size.X, imagePosStart.Y + Size.Y);

            // 背景阴影
            drawList.AddRectFilled(imagePosStart, thumbnailPosEnd + new Vector2(3, 3),
                ImGui.ColorConvertFloat4ToU32(new Vector4(0.01f, 0.01f, 0.01f, 1.0f)), 4.0f);

            // 裁剪区域
            ImGui.PushClipRect(p0, p1, true);

            // 图标背景
            drawList.AddRectFilled(imagePosStart, imagePosEnd,
                ImGui.ColorConvertFloat4ToU32(new Vector4(0, 0, 0, 1.0f)));

            // 图标
            string icon = isDir ? "📁" : GetFileIcon(name);
            var iconSize = ImGui.CalcTextSize(icon);
            var iconPos = new Vector2(
                imagePosStart.X + (ImageSize.X - iconSize.X) * 0.5f,
                imagePosStart.Y + (ImageSize.Y - iconSize.Y) * 0.5f);
            drawList.AddText(iconPos, ImGui.ColorConvertFloat4ToU32(Vector4.One), icon);

            // 文本背景
            drawList.AddRectFilled(textPosStart, textPosEnd,
                ImGui.ColorConvertFloat4ToU32(new Vector4(0.04f, 0.04f, 0.04f, 1.0f)));

            // 文件名
            drawList.AddText(textPosStart, ImGui.ColorConvertFloat4ToU32(Vector4.One), name);

            // 资产类型
            if (!isDir && !string.IsNullOrEmpty(assetType))
            {
                drawList.AddText(assetTypeStart, AssetTypeColor, assetType);
            }

            // 边框高亮
            if (ImGui.IsItemHovered())
            {
                drawList.AddRect(imagePosStart, thumbnailPosEnd,
                    ImGui.ColorConvertFloat4ToU32(new Vector4(0.39f, 0.39f, 0.39f, 0.78f)), 3.0f, 0, 2.0f);
            }

            ImGui.PopClipRect();
        }

        private string GetFileIcon(string fileName)
        {
            var ext = Path.GetExtension(fileName).ToLower();
            return ext switch
            {
                ".png" or ".jpg" or ".jpeg" or ".bmp" or ".tga" => "🖼️",
                ".fbx" or ".obj" or ".gltf" or ".glb" => "🧊",
                ".wav" or ".mp3" or ".ogg" => "🎵",
                ".mp4" or ".avi" or ".mov" => "🎬",
                ".cs" or ".cpp" or ".h" or ".py" => "📝",
                ".json" or ".xml" or ".yaml" or ".yml" => "📋",
                ".txt" or ".md" => "📄",
                ".shader" or ".hlsl" or ".glsl" => "☀️",
                ".scene" => "🗺️",
                ".prefab" => "🧊",
                ".mat" => "📋",
                _ => "📄"
            };
        }
    }
}
