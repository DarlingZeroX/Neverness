using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Public;
using System.Numerics;
using Neverness.Editor.Framework.Private.Core;

namespace Neverness.Editor.Framework.Private.Panel.ContentBrowser;

public class ContentBrowserPanel : IEditorPanel
{
    private bool _isOpen = true;

    private Core.ContentBrowser _contentBrowser;

    public ContentBrowserPanel()
    {
        if (Core.ContentBrowser.Instance != null) _contentBrowser = Core.ContentBrowser.Instance;
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
                // 注意：ImGuiEx 类在 Hexa.NET 中可能需要你手动实现或引用对应的 UI 扩展库
                // 假设你有类似的逻辑实现：
                /*
                RectFilledMultiColor bg = new RectFilledMultiColor();
                ImVec2 region = ImGui.GetContentRegionAvail();
                bg.SetRegionAutoOffest(region, 0.0f, 0.0f);
                bg.SetColRight(new Vector4(0.0f, 0.0f, 0.0f, 0.431f));
                bg.WindowDraw();
                */

                DrawBrowserHeader();

                ImGui.EndMenuBar();
            }

            // Top shadow
            //RectFilledMultiColor bgShadow = new RectFilledMultiColor();
            //ImVec2 regionAvail = ImGui.GetContentRegionAvail();
            //bgShadow.SetRegionAutoOffest(regionAvail, 0.0f, 0.0f, -0.0f, -regionAvail.Y + 6);
            //bgShadow.SetColTop(new Vector4(0.0f, 0.0f, 0.0f, 0.731f));
            //bgShadow.WindowDraw();

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
                //DrawContentBrowser();
                //ContentBrowserContextMenu(m_pContentBrowser.GetCurrentBrowserDirectory());
                ImGui.EndChild();
            }
        }

        ImGui.End();
    }

    private void DrawContentBrowser()
    {

    }

    public void DrawRootDirectoryTree()
    {
        // 目录右边的分割阴影
        // 注意：需确保 RectFilledMultiColor 已在 C# 中实现并包含相应的 SetRegionAutoOffest 方法
        //RectFilledMultiColor shadow = new RectFilledMultiColor();
        //Vector2 region = ImGui.GetContentRegionAvail();
        //shadow.SetRegionAutoOffest(region, region.X - 20.0f, 10.0f);
        //shadow.SetColRight(new Vector4(0.0f, 0.0f, 0.0f, 0.431f));
        //shadow.WindowDraw();

        // 总 Assets 目录
        // ImGuiTreeNodeFlags_DefaultOpen 对应 ImGuiTreeNodeFlags.DefaultOpen
        if (ImGui.CollapsingHeader("Assets##ContentBrowserPanel", ImGuiTreeNodeFlags.DefaultOpen))
        {
            // 点击打开 Assets 目录
            //DirectoryItemFunction(_contentBrowser.GetDirectoryTreeRootNode());

            // 遍历并绘制子目录
            foreach (var child in _contentBrowser.GetDirectoryTreeRootNode().Directories)
            {
                DrawDirectoryTree(child);
            }
        }
    }

    public void DrawDirectoryTree(ContentDirectory node)
    {
        // C# 中使用 PushID / PopID 实现 ScopedID 的逻辑
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

        // 点击打开目录 (处理点击事件)
        //DirectoryItemFunction(node);

        ImGui.PopID();
    }

    public void DrawBrowserHeader()
    {
        // 1. 实现 ScopedStyleColor，确保在作用域结束时自动还原颜色
        using (new ImGuiStyleColor(ImGuiCol.Button, new Vector4(0, 0, 0, 0)))
        {
            Vector2 thumbnailSize = new Vector2(25.0f, 24.0f);

            // 2. 路径判断逻辑
            string currentDir = _contentBrowser.GetCurrentBrowserDirectory();
            string projectDir = _contentBrowser.GetProjectDirectory();
            string relative = Path.GetRelativePath(projectDir, currentDir);

            // 如果不是根目录，显示返回按钮
            if (relative != ".")
            {
                if (ImGui.Button(FontAwesome5Pro.ChevronCircleLeft + "##Back", thumbnailSize))
                {
                    // 获取上级目录
                    string parentDir = Directory.GetParent(currentDir)?.FullName ?? string.Empty;

                    // 检查目录是否存在且是否仍在项目根目录下
                    if (Directory.Exists(parentDir) && parentDir.StartsWith(projectDir, StringComparison.OrdinalIgnoreCase))
                    {
                        _contentBrowser.OpenDirectory(parentDir);
                    }
                }
                ImGui.SameLine();
            }

            // 3. 刷新按钮
            if (ImGui.Button(FontAwesome5Pro.Redo + "##Refresh", thumbnailSize))
            {
                //RefreshDirectory();
            }
            ImGui.SameLine();

            // 4. 绘制路径栏
            DrawPath(currentDir);
        }
    }

    // 辅助类：C# 版 ScopedStyleColor
    public struct ImGuiStyleColor : IDisposable
    {
        public ImGuiStyleColor(ImGuiCol idx, Vector4 col) => ImGui.PushStyleColor(idx, col);
        public void Dispose() => ImGui.PopStyleColor();
    }

    public void DrawPath(string path)
    {
        // 获取项目根目录路径字符串
        string projectDir = _contentBrowser.GetProjectDirectory();

        // 1. 获取相对路径
        // 使用 Uri 或 Path.GetRelativePath ( .NET Core 3.0+ 支持)
        string relativePath = Path.GetRelativePath(projectDir, path);

        ImGui.Separator();

        // 2. 将相对路径按路径分隔符分割
        // 注意：Windows下使用 '\\'，但 Path.Combine 和 Path.GetRelativePath 会自动处理平台差异
        string[] splitItems = relativePath.Split(Path.DirectorySeparatorChar, StringSplitOptions.RemoveEmptyEntries);

        int pathPos = 0;

        foreach (var item in splitItems)
        {
            // 计算当前段在 relativePath 中的偏移位置
            // 注意：这里需要手动加 1 补偿分隔符
            pathPos += item.Length + 1;

            if (item == ".")
                continue;

            // 3. 绘制路径按钮
            if (ImGui.Button(item, Vector2.Zero))
            {
                // 截取当前点击层级对应的相对路径
                string subPath = relativePath.Substring(0, Math.Min(pathPos - 1, relativePath.Length));

                // 拼接回完整的物理路径
                string targetPath = Path.Combine(projectDir, subPath);
                _contentBrowser.OpenDirectory(targetPath);
                break;
            }

            // 4. 在按钮右侧绘制分隔图标
            ImGui.SameLine();
            ImGui.Text(FontAwesome5Pro.ChevronRight);
            ImGui.SameLine();
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
