using Hexa.NET.ImGui;
using System.Numerics;

namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// 纹理查看器窗口。支持缩放、平移、Checkerboard 背景、Mipmap 预览。
///
/// 多实例支持：可同时打开多个 TextureViewer 查看不同纹理。
/// 鼠标交互：滚轮缩放，中键拖拽平移。
///
/// 用法:
/// <code>
/// var viewer = windowManager.OpenWindow&lt;ImTextureViewerWindow&gt;();
/// viewer.SetTexture(imGuiTextureHandle, new Vector2(width, height), mipCount);
/// </code>
/// </summary>
public sealed class ImTextureViewerWindow : ImWindow
{
    // ── 纹理数据 ──
    private ulong m_TextureHandle;
    private Vector2 m_TextureSize;
    private int m_MipCount = 1;

    // ── 视图状态 ──
    private float m_Zoom = 1.0f;
    private Vector2 m_PanOffset;
    private bool m_FitToWindow = true;

    // ── 显示选项 ──
    private bool m_ShowCheckerboard = true;
    private int m_SelectedMipLevel;

    // ── 资产关联 ──
    private string? m_AssetName;

    /// <summary>关联的资产名称（用于窗口标题显示）。</summary>
    public string? AssetName
    {
        get => m_AssetName;
        set
        {
            m_AssetName = value;
            UpdateTitle();
        }
    }

    /// <summary>关联的资产 GUID 十六进制字符串（由 Opener 设置，用于 AssetEditorManager 映射）。</summary>
    public string? AssetGuidHex { get; set; }

    public ImTextureViewerWindow() : base("Texture Viewer")
    {
        BehaviorFlags = ImWindowBehaviorFlags.MultiInstance | ImWindowBehaviorFlags.Default;
        Flags = ImGuiWindowFlags.MenuBar | ImGuiWindowFlags.NoScrollbar;
    }

    /// <summary>设置要查看的纹理。</summary>
    /// <param name="textureHandle">ImGui 纹理句柄（来自 TextureInterop）。</param>
    /// <param name="size">纹理像素尺寸。</param>
    /// <param name="mipCount">Mipmap 级别数。</param>
    public void SetTexture(ulong textureHandle, Vector2 size, int mipCount = 1)
    {
        m_TextureHandle = textureHandle;
        m_TextureSize = size;
        m_MipCount = Math.Max(1, mipCount);
        m_Zoom = 1.0f;
        m_PanOffset = Vector2.Zero;
        m_FitToWindow = true;
        m_SelectedMipLevel = 0;

        UpdateTitle();
    }

    private void UpdateTitle()
    {
        var w = (int)m_TextureSize.X;
        var h = (int)m_TextureSize.Y;
        if (m_AssetName != null)
            Title = $"{m_AssetName} - {w}x{h}";
        else if (w > 0 && h > 0)
            Title = $"Texture Viewer - {w}x{h}";
        else
            Title = "Texture Viewer";
    }

    /// <summary>当前是否已设置纹理。</summary>
    public bool HasTexture => m_TextureHandle != 0;

    /// <summary>当前缩放倍率。</summary>
    public float Zoom => m_Zoom;

    // ── 菜单栏 ──

    protected override bool HasMenuBar() => true;

    protected override void OnRenderMenuBar()
    {
        if (ImGui.BeginMenu("View"))
        {
            ImGui.MenuItem("Fit to Window", "", ref m_FitToWindow);
            ImGui.MenuItem("Checkerboard", "", ref m_ShowCheckerboard);
            ImGui.EndMenu();
        }

        if (m_MipCount > 1 && ImGui.BeginMenu("Mip Level"))
        {
            for (int i = 0; i < m_MipCount; i++)
            {
                if (ImGui.MenuItem($"Mip {i}", "", i == m_SelectedMipLevel))
                    m_SelectedMipLevel = i;
            }
            ImGui.EndMenu();
        }

        // 右侧信息
        ImGui.SameLine(ImGui.GetWindowWidth() - 220);
        ImGui.TextDisabled($"Zoom: {m_Zoom:F2}x  |  {(int)m_TextureSize.X} x {(int)m_TextureSize.Y}");
    }

    // ── 内容渲染 ──

    protected override void OnRender()
    {
        if (m_TextureHandle == 0)
        {
            var center = ImGui.GetContentRegionAvail();
            ImGui.SetCursorPosX(center.X * 0.5f - 60);
            ImGui.SetCursorPosY(center.Y * 0.5f);
            ImGui.TextDisabled("No texture loaded.");
            return;
        }

        var available = ImGui.GetContentRegionAvail();
        if (available.X <= 0 || available.Y <= 0) return;

        var drawList = ImGui.GetWindowDrawList();
        var cursor = ImGui.GetCursorScreenPos();

        // Checkerboard 背景
        if (m_ShowCheckerboard)
        {
            DrawCheckerboard(drawList, cursor, available);
        }

        // 计算显示尺寸
        Vector2 displaySize = CalculateDisplaySize(available);
        var displayPos = cursor + (available - displaySize) * 0.5f + m_PanOffset;

        // 绘制纹理
        unsafe
        {
            drawList.AddImage(
                new ImTextureRef(null, m_TextureHandle),
                displayPos,
                displayPos + displaySize);
        }

        // 鼠标交互
        HandleMouseInteraction(available);
    }

    private Vector2 CalculateDisplaySize(Vector2 available)
    {
        if (m_FitToWindow)
        {
            float scaleX = available.X / m_TextureSize.X;
            float scaleY = available.Y / m_TextureSize.Y;
            float scale = Math.Min(scaleX, scaleY);
            m_Zoom = scale;
            return m_TextureSize * scale;
        }

        return m_TextureSize * m_Zoom;
    }

    private void HandleMouseInteraction(Vector2 available)
    {
        if (!ImGui.IsWindowHovered()) return;

        // 滚轮缩放
        float wheel = ImGui.GetIO().MouseWheel;
        if (wheel != 0)
        {
            m_FitToWindow = false;
            float factor = wheel > 0 ? 1.15f : 1.0f / 1.15f;
            m_Zoom = Math.Clamp(m_Zoom * factor, 0.05f, 100.0f);
        }

        // 中键拖拽平移
        if (ImGui.IsMouseDragging(ImGuiMouseButton.Middle))
        {
            m_FitToWindow = false;
            var delta = ImGui.GetMouseDragDelta(ImGuiMouseButton.Middle);
            m_PanOffset += delta;
            ImGui.ResetMouseDragDelta(ImGuiMouseButton.Middle);
        }

        // 双击中键重置视图
        if (ImGui.IsMouseDoubleClicked(ImGuiMouseButton.Middle))
        {
            m_FitToWindow = true;
            m_PanOffset = Vector2.Zero;
            m_Zoom = 1.0f;
        }
    }

    // ── Checkerboard 绘制 ──

    private static void DrawCheckerboard(ImDrawListPtr drawList, Vector2 pos, Vector2 size)
    {
        const float cell = 12f;
        uint colorA = ImGui.GetColorU32(new Vector4(0.25f, 0.25f, 0.25f, 1f));
        uint colorB = ImGui.GetColorU32(new Vector4(0.35f, 0.35f, 0.35f, 1f));

        for (float y = 0; y < size.Y; y += cell)
        {
            for (float x = 0; x < size.X; x += cell)
            {
                bool even = ((int)(x / cell) + (int)(y / cell)) % 2 == 0;
                var p0 = pos + new Vector2(x, y);
                var p1 = pos + new Vector2(
                    Math.Min(x + cell, size.X),
                    Math.Min(y + cell, size.Y));
                drawList.AddRectFilled(p0, p1, even ? colorA : colorB);
            }
        }
    }
}
