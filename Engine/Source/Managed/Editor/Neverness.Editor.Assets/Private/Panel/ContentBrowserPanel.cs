using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Private.Menu;
using Neverness.Editor.Framework.Public;
using System.Numerics;
using Neverness.Editor.Assets.Private.Core;
using Neverness.Editor.Assets.Private.Context;

namespace Neverness.Editor.Assets.Private.Panel;

public class ContentBrowserPanel : IEditorPanel
{
    private bool _isOpen = true;

    private ContentBrowser _contentBrowser;

    public ContentBrowserPanel()
    {
        if (ContentBrowser.Instance != null) _contentBrowser = ContentBrowser.Instance;
    }

    public void OnUpdate(float delta)
    {
        throw new NotImplementedException();
    }

    public void OnFixedUpdate()
    {
        throw new NotImplementedException();
    }

    public bool IsAsync()
    {
        throw new NotImplementedException();
    }

    public void OnGUI()
    {
        if (!_isOpen)
            return;

        // Hexa.NET 中的 Begin 使用 string 即可
        if (ImGui.Begin(GetWindowFullName(), ImGuiWindowFlags.NoScrollbar | ImGuiWindowFlags.MenuBar))
        {
            // 菜单栏 Menu bar
            if (ImGui.BeginMenuBar())
            {
                DrawBrowserHeader();

                ImGui.EndMenuBar();
            }

            // 左侧目录树
            {
                ImGui.BeginChild("Content Browser Left", new Vector2(200, 0), ImGuiChildFlags.Borders | ImGuiChildFlags.ResizeX);
                DrawRootDirectoryTree();
                ImGui.EndChild();
            }

            ImGui.SameLine();

            // 右侧内容浏览器
            {
                ImGui.BeginChild("Content Browser Right", new Vector2(0, 0), ImGuiChildFlags.Borders);
                DrawContentBrowser();
                ShowBackgroundContextMenu(_contentBrowser.GetCurrentBrowserDirectory());
                ImGui.EndChild();
            }
        }

        ImGui.End();
    }

    /// <summary>
    /// 空白区域右键上下文菜单——通过 ContextMenuManager 贡献者模式渲染。
    /// 外部可通过 <see cref="ContentBrowserContextMenu"/> 扩展。
    /// </summary>
    private void ShowBackgroundContextMenu(string path)
    {
        var ctx = ContextMenuManager.Instance;

        // 注入运行时上下文，供贡献者命令回调读取
        ctx.SetContext(ContentBrowserContextMenu.KeyPath, path);
        ctx.SetContext(ContentBrowserContextMenu.KeyContentBrowser, _contentBrowser);

        ctx.RenderWindowContextMenu(ContentBrowserContextMenu.BackgroundId);
    }

    /// <summary>
    /// 项目右键上下文菜单——通过 ContextMenuManager 贡献者模式渲染。
    /// 外部可通过 <see cref="ContentBrowserContextMenu"/> 扩展。
    /// </summary>
    private void ShowItemContextMenu(ContentItem item)
    {
        var ctx = ContextMenuManager.Instance;

        // 注入运行时上下文，供贡献者命令回调读取
        ctx.SetContext(ContentBrowserContextMenu.KeyItem, item);
        ctx.SetContext(ContentBrowserContextMenu.KeyContentBrowser, _contentBrowser);

        ctx.RenderItemContextMenu(ContentBrowserContextMenu.ItemId);
    }

    private void DrawContentBrowser()
    {
        _contentBrowser.ClearRefreshedFlag();
        // 使用 using 语句替代 C++ 的 Scoped 作用域管理
        using (new ImGuiEx.StyleColor(ImGuiCol.Button, new Vector4(0, 0, 0, 0)))
        using (new ImGuiEx.StyleVar(ImGuiStyleVar.ItemSpacing, new Vector2(4, 8)))
        {
            var thumbnail = ContentBrowserFileUIBox.Instance;
            Vector2 windowPos = ImGui.GetWindowPos();
            Vector2 windowSize = ImGui.GetWindowSize();
            Vector2 windowEndPos = new Vector2(windowPos.X + windowSize.X, windowPos.Y + windowSize.Y);

            // 计算列数
            float panelWidth = ImGui.GetContentRegionAvail().X;
            int columnCount = (int)(panelWidth / thumbnail.CellSize);
            if (columnCount < 1) columnCount = 1;

            ImGui.Columns(columnCount, 0, false);
            ImDrawListPtr drawList = ImGui.GetWindowDrawList();

            // 定义绘制逻辑的 Action
            void DrawItemFunction(ref ContentItem item, bool isDir)
            {
                ImGui.PushID(item.GetHashCode()); // 使用 HashCode 或唯一 ID 替代指针

                if (item.Renaming)
                    ImGui.InvisibleButton(item.AbsolutePath, thumbnail.ImageSize);
                else
                    ImGui.InvisibleButton(item.AbsolutePath, thumbnail.Size);

                Vector2 p0 = ImGui.GetItemRectMin();
                Vector2 p1 = new Vector2(p0.X + thumbnail.Size.X, p0.Y + thumbnail.Size.Y);

                // 裁剪检查
                if (p0.Y > windowEndPos.Y || p1.Y < windowPos.Y)
                {
                    ImGui.PopID();
                    return;
                }

                thumbnail.Draw(_contentBrowser, drawList, ref item, p0, p1);
                ImGui.PopID();
            }

            // 绘制目录
            foreach (var item in _contentBrowser.GetCurrentDirectoryNode().Directories)
            {
                ContentItem tempItem = item; // 闭包处理
                DrawItemFunction(ref tempItem, true);
                if (_contentBrowser.IsRefreshed()) break;

                ImGui.PushID(item.GetHashCode());
                if (ItemFunction(ref tempItem, true)) { ImGui.PopID(); break; }
                ImGui.PopID();

                ImGui.NextColumn();
            }

            // 绘制文件
            foreach (var item in _contentBrowser.GetCurrentDirectoryNode().Files)
            {
                ContentItem tempItem = item;
                DrawItemFunction(ref tempItem, false);
                if (_contentBrowser.IsRefreshed()) break;

                ImGui.PushID(item.GetHashCode());
                if (ItemFunction(ref tempItem, false)) { ImGui.PopID(); break; }
                ImGui.PopID();

                ImGui.NextColumn();
            }

            ImGui.Columns(1); // 记得重置列数
        }
    }

    private bool ItemFunction(ref ContentItem item, bool isDir)
    {
        if (ImGui.IsItemHovered())
        {
            ImGui.BeginTooltip();
            ImGui.Text(item.AbsolutePath.ToString());
            ImGui.EndTooltip();

            // 单击
            if (ImGui.IsMouseClicked(ImGuiMouseButton.Left))
            {
                _contentBrowser.SelectAll(false);
                item.Selected = true;
            }
            // 双击
            if (ImGui.IsMouseDoubleClicked(ImGuiMouseButton.Left))
            {
                if (isDir)
                {
                    _contentBrowser.OpenDirectory(item.AbsolutePath);
                    return true;
                }
                else
                {
                    //AssetEditor.Get().OpenAsset(item.Path, item.MetaData);
                }
            }
        }

        ShowItemContextMenu(item);

        return false;
    }

    public void DrawRootDirectoryTree()
    {
        // 总 Assets 目录
        if (ImGui.CollapsingHeader("Assets##ContentBrowserPanel", ImGuiTreeNodeFlags.DefaultOpen))
        {
            // 点击打开 Assets 目录
            DirectoryItemFunction(_contentBrowser.GetDirectoryTreeRootNode());

            // 遍历并绘制子目录
            foreach (var child in _contentBrowser.GetDirectoryTreeRootNode().Directories)
            {
                DrawDirectoryTree(child);
            }
        }
    }

    public void DrawDirectoryTree(ContentDirectory node)
    {
        ImGui.PushID(node.AbsolutePath);

        string nodeName = $"{FontAwesome5Pro.Folder} {node.Name}";

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
        DirectoryItemFunction(node);

        ImGui.PopID();
    }

    private void DirectoryItemFunction(ContentDirectory node)
    {
        if (ImGui.IsItemHovered() && ImGui.IsMouseClicked(ImGuiMouseButton.Left))
        {
            _contentBrowser.OpenDirectory(node.AbsolutePath);
        }
    }

    public static bool IsSubPath(
        string basePath,
        string targetPath)
    {
        var baseFullPath = Path.GetFullPath(basePath)
                               .TrimEnd(Path.DirectorySeparatorChar, Path.AltDirectorySeparatorChar)
                           + Path.DirectorySeparatorChar;

        var targetFullPath = Path.GetFullPath(targetPath);

        return targetFullPath.StartsWith(
            baseFullPath,
            OperatingSystem.IsWindows()
                ? StringComparison.OrdinalIgnoreCase
                : StringComparison.Ordinal);
    }

    public void DrawBrowserHeader()
    {
        using (new ImGuiEx.StyleColor(ImGuiCol.Button, new Vector4(0, 0, 0, 0)))
        {
            Vector2 thumbnailSize = new Vector2(25.0f, 24.0f);

            string currentDir = _contentBrowser.GetCurrentBrowserDirectory();
            string projectDir = _contentBrowser.GetProjectDirectory();
            string relative = Path.GetRelativePath(projectDir, currentDir);

            // 如果不是根目录，显示返回按钮
            if (relative != ".")
            {
                if (ImGui.Button(FontAwesome5Pro.ChevronCircleLeft + "##Back", thumbnailSize))
                {
                    string parentDir = Directory.GetParent(currentDir)?.FullName ?? string.Empty;

                    if (Directory.Exists(parentDir) && IsSubPath(projectDir, currentDir))
                    {
                        _contentBrowser.OpenDirectory(parentDir);
                    }
                }
            }

            // 刷新按钮
            if (ImGui.Button(FontAwesome5Pro.Redo + "##Refresh", thumbnailSize))
            {
                //RefreshDirectory();
            }

            // 绘制路径栏
            DrawPath(currentDir);
        }
    }

    public void DrawPath(string path)
    {
        string projectDir = _contentBrowser.GetProjectDirectory();
        string relativePath = Path.GetRelativePath(projectDir, path);

        ImGui.Separator();

        string[] splitItems = relativePath.Split(Path.DirectorySeparatorChar, StringSplitOptions.RemoveEmptyEntries);

        int pathPos = 0;

        foreach (var item in splitItems)
        {
            pathPos += item.Length + 1;

            if (item == ".")
                continue;

            if (ImGui.Button(item, Vector2.Zero))
            {
                string subPath = relativePath.Substring(0, Math.Min(pathPos - 1, relativePath.Length));
                string targetPath = Path.Combine(projectDir, subPath);
                _contentBrowser.OpenDirectory(targetPath);
                break;
            }

            ImGui.Text(FontAwesome5Pro.ChevronRight);
        }
    }

    public string GetWindowFullName()
    {
        return FontAwesome5Pro.Window + " " + GetWindowName();
    }

    public string GetWindowName() => "Content Browser";

    public void OpenWindow(bool open) => _isOpen = open;

    public bool IsWindowOpened() => _isOpen;
}
