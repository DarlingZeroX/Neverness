using Hexa.NET.ImGui;
using Neverness.Editor.Framework.Interface;
using Neverness.Editor.Framework.Public;
using System.Numerics;
using Neverness.Editor.Framework.Private.Core;

namespace Neverness.Editor.Framework.Private.Panel.ContentBrowser;

using System.Numerics;
using Hexa.NET.ImGui;

public class ContentBrowserFileUIBox
{
    // 单例模式实现
    private static ContentBrowserFileUIBox _instance;
    public static ContentBrowserFileUIBox Instance => _instance ??= new ContentBrowserFileUIBox();

    public float ThumbnailTextSizeY;
    public float Padding;
    public float ThumbnailImageSizeY;

    public float CellSize;
    public Vector2 Size;
    public Vector2 TextSize;
    public Vector2 ImageSize;
    public uint AssetTypeColor;

    public ContentBrowserFileUIBox()
    {
        ThumbnailTextSizeY = 35.0f;
        ThumbnailImageSizeY = 90.0f;
        Padding = 6.0f;

        Size = new Vector2(ThumbnailImageSizeY, ThumbnailImageSizeY + ThumbnailTextSizeY);
        CellSize = Size.X + Padding;

        TextSize = new Vector2(CellSize, ThumbnailTextSizeY);
        ImageSize = new Vector2(Size.X, Size.Y - ThumbnailTextSizeY);
        AssetTypeColor = ImGui.ColorConvertFloat4ToU32(new Vector4(0.39f, 0.39f, 0.39f, 1.0f)); // 100, 100, 100
    }

    public void Draw(Core.ContentBrowser browser, ImDrawListPtr drawList, ref ContentItem item, Vector2 p0, Vector2 p1, bool isDir = false)
    {
        Vector2 textPosStart = new Vector2(p0.X, p0.Y + Size.Y - ThumbnailTextSizeY);
        Vector2 assetTypeStart = new Vector2(p0.X, p0.Y + Size.Y - 18);
        Vector2 textPosEnd = new Vector2(textPosStart.X + TextSize.X, textPosStart.Y + TextSize.Y);

        Vector2 imagePosStart = new Vector2(p0.X, p0.Y);
        Vector2 imagePosEnd = new Vector2(imagePosStart.X + ImageSize.X, imagePosStart.Y + ImageSize.Y);
        Vector2 thumbnailPosEnd = new Vector2(imagePosStart.X + Size.X, imagePosStart.Y + Size.Y);

        // 背景阴影
        drawList.AddRectFilled(imagePosStart, thumbnailPosEnd + new Vector2(3, 3), ImGui.ColorConvertFloat4ToU32(new Vector4(0.01f, 0.01f, 0.01f, 1.0f)), 4.0f);

        // 裁剪区域
        ImGui.PushClipRect(p0, p1, true);

        drawList.AddRectFilled(imagePosStart, imagePosEnd, ImGui.ColorConvertFloat4ToU32(new Vector4(0, 0, 0, 1.0f)));

        //if (item.IconView != IntPtr.Zero)
        //    drawList.AddImage(item.IconView, imagePosStart, imagePosEnd);

        drawList.AddRectFilled(textPosStart, textPosEnd, ImGui.ColorConvertFloat4ToU32(new Vector4(0.04f, 0.04f, 0.04f, 1.0f)));

        if (item.Renaming)
        {
            // 设置输入框位置，注意：ImGui 输入框通常需要在布局中处理，这里简单处理逻辑
            ImGui.SetCursorScreenPos(textPosStart);
            if (ImGui.InputText("##Renaming", ref item.Name, 256, ImGuiInputTextFlags.EnterReturnsTrue))
            {
                item.Renaming = false;
                browser.RenameDirectoryItem(item, item.Name);
            }

            // 结束重命名判断
            if (ImGui.IsItemDeactivated() || (!ImGui.IsItemHovered() && ImGui.IsMouseClicked(ImGuiMouseButton.Left)))
            {
                item.Renaming = false;
                browser.RenameDirectoryItem(item, item.Name);
            }
        }
        else
        {
            drawList.AddText(textPosStart, ImGui.ColorConvertFloat4ToU32(Vector4.One), item.Name);

            //if (isDir = false)
            //    drawList.AddText(assetTypeStart, AssetTypeColor, item.AssetType);
        }

        // 边框高亮
        if (ImGui.IsItemHovered() || item.Selected)
            drawList.AddRect(imagePosStart, thumbnailPosEnd, ImGui.ColorConvertFloat4ToU32(new Vector4(0.39f, 0.39f, 0.39f, 0.78f)), 3.0f, 0, 2.0f);

        ImGui.PopClipRect();
    }
}
