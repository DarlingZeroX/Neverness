using Hexa.NET.ImGui;
using System.Numerics;

namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// ImGui 通知渲染器。纯静态方法，接收数据并绘制 Toast 通知。
///
/// 定位在视口右下角，自下而上堆叠。
/// 支持淡入 (0.25s) / 淡出 (0.15s) 动画。
/// 使用 DrawList 自绘背景和色条。
/// </summary>
public static class ImNotificationRenderer
{
    private const float Padding = 10f;
    private const float Width = 300f;
    private const float Gap = 6f;
    private const float AnimateInDuration = 0.25f;
    private const float AnimateOutDuration = 0.15f;
    private const float CornerRadius = 4f;

    // 级别颜色
    private static readonly Vector4[] LevelColors =
    [
        new(0.2f, 0.5f, 0.9f, 0.9f),  // Info — 蓝
        new(0.2f, 0.8f, 0.3f, 0.9f),  // Success — 绿
        new(0.9f, 0.7f, 0.1f, 0.9f),  // Warning — 黄
        new(0.9f, 0.2f, 0.2f, 0.9f),  // Error — 红
    ];

    /// <summary>渲染通知列表。应在每帧末尾调用。</summary>
    public static void Render(IReadOnlyList<NotificationItem> notifications)
    {
        if (notifications.Count == 0) return;

        var viewport = ImGui.GetMainViewport();
        var viewportPos = viewport.WorkPos;
        var viewportSize = viewport.WorkSize;

        // 右下角锚点
        var anchorPos = new Vector2(
            viewportPos.X + viewportSize.X - Width - Padding,
            viewportPos.Y + viewportSize.Y - Padding);

        ImGui.SetNextWindowPos(anchorPos, ImGuiCond.Always, new Vector2(0, 1));
        ImGui.SetNextWindowSize(new Vector2(Width, 0), ImGuiCond.Always);
        ImGui.SetNextWindowBgAlpha(0.0f); // 透明背景，自己画

        ImGuiWindowFlags flags =
            ImGuiWindowFlags.NoDecoration |
            ImGuiWindowFlags.NoInputs |
            ImGuiWindowFlags.NoNav |
            ImGuiWindowFlags.NoFocusOnAppearing |
            ImGuiWindowFlags.NoSavedSettings |
            ImGuiWindowFlags.NoDocking;

        if (!ImGui.Begin("##Notifications", flags)) { ImGui.End(); return; }

        var drawList = ImGui.GetWindowDrawList();
        var now = DateTime.UtcNow;

        for (int i = notifications.Count - 1; i >= 0; i--)
        {
            var n = notifications[i];
            float age = (float)(now - n.CreatedAt).TotalSeconds;
            float remaining = (float)(n.Duration - (now - n.CreatedAt)).TotalSeconds;

            // 跳过已过期的
            if (remaining <= 0) continue;

            // 动画 alpha
            float alpha = CalculateAlpha(age, remaining);

            var levelColor = LevelColors[(int)n.Level];
            levelColor.W *= alpha;

            var cursor = ImGui.GetCursorScreenPos();

            // 计算通知高度
            var textSize = ImGui.CalcTextSize(n.Message, false, Width - 24);
            float notifHeight = 36f + Math.Max(0, textSize.Y - 14);

            // 背景
            drawList.AddRectFilled(
                cursor,
                cursor + new Vector2(Width, notifHeight),
                ImGui.GetColorU32(new Vector4(0.12f, 0.12f, 0.12f, 0.92f * alpha)),
                CornerRadius);

            // 左侧色条
            drawList.AddRectFilled(
                cursor,
                cursor + new Vector2(4, notifHeight),
                ImGui.GetColorU32(levelColor),
                CornerRadius);

            // 标题 + 消息
            ImGui.PushStyleVar(ImGuiStyleVar.Alpha, alpha);
            ImGui.Indent(14);
            ImGui.TextColored(levelColor, n.Title);
            ImGui.TextWrapped(n.Message);
            ImGui.Unindent(14);
            ImGui.PopStyleVar();

            ImGui.Dummy(new Vector2(0, Gap));
        }

        ImGui.End();
    }

    private static float CalculateAlpha(float age, float remaining)
    {
        if (age < AnimateInDuration)
            return age / AnimateInDuration;
        if (remaining < AnimateOutDuration)
            return remaining / AnimateOutDuration;
        return 1.0f;
    }
}
