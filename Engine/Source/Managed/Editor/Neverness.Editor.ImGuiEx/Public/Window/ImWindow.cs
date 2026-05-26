using Hexa.NET.ImGui;
using System.Numerics;

namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// ImGui 窗口抽象基类。
///
/// 封装 ImGui.Begin/End 生命周期、焦点跟踪、Docking 支持、菜单栏。
/// 子类实现 OnRender() 绘制窗口内容，无需直接调用 ImGui.Begin/End。
///
/// 多实例支持：每个实例构造时生成唯一 Guid，GetImGuiId() 返回 "Title##GUID_HEX"，
/// 保证同类型多实例窗口在 ImGui 中不合并。
/// </summary>
public abstract class ImWindow : IImWindow, IDisposable
{
    // ── 状态 ──

    private bool m_WasFocused;
    private bool m_IsOpen = true;
    private bool m_IsVisible = true;
    private bool m_Disposed;

    // ── 属性 ──

    /// <inheritdoc />
    public Guid WindowId { get; }

    /// <inheritdoc />
    public string Title { get; set; }

    /// <inheritdoc />
    public bool IsOpen => m_IsOpen;

    /// <inheritdoc />
    public bool IsFocused { get; private set; }

    /// <inheritdoc />
    public bool IsVisible
    {
        get => m_IsVisible;
        set => m_IsVisible = value;
    }

    /// <inheritdoc />
    public ImGuiWindowFlags Flags { get; set; } = ImGuiWindowFlags.None;

    /// <inheritdoc />
    public ImWindowBehaviorFlags BehaviorFlags { get; set; } = ImWindowBehaviorFlags.Default;

    /// <inheritdoc />
    public uint DockId { get; set; }

    // ── 构造 ──

    /// <summary>创建窗口，自动生成唯一 Guid。</summary>
    protected ImWindow(string title)
    {
        WindowId = Guid.NewGuid();
        Title = title ?? throw new ArgumentNullException(nameof(title));
    }

    // ── ImGui ID ──

    /// <inheritdoc />
    public string GetImGuiId() => $"{Title}##{WindowId:N}";

    // ── 生命周期（virtual，子类可覆写）──

    /// <inheritdoc />
    public virtual void OnOpen() { }

    /// <inheritdoc />
    public virtual void OnClose() { }

    /// <inheritdoc />
    public virtual void OnFocus() { }

    /// <inheritdoc />
    public virtual void OnLostFocus() { }

    /// <inheritdoc />
    public virtual void OnUpdate(float deltaTime) { }

    // ── 渲染（子类必须/可覆写）──

    /// <summary>子类实现：绘制窗口内容区域。</summary>
    protected abstract void OnRender();

    /// <summary>子类可覆写：绘制窗口菜单栏。</summary>
    protected virtual void OnRenderMenuBar() { }

    /// <summary>是否显示菜单栏。默认由 BehaviorFlags.HasMenuBar 决定。</summary>
    protected virtual bool HasMenuBar()
        => BehaviorFlags.HasFlag(ImWindowBehaviorFlags.HasMenuBar);

    // ── 模板方法 Render ──

    /// <summary>
    /// 框架每帧调用。控制 ImGui.Begin/End 配对、焦点跟踪、Docking。
    /// 子类不应覆写此方法，应实现 OnRender()。
    /// </summary>
    public void Render()
    {
        if (!m_IsVisible || !m_IsOpen) return;

        // ── Docking 预设 ──
        if (DockId != 0)
        {
            unsafe
            {
                ImGui.SetNextWindowDockID(DockId);
            }
        }

        if (BehaviorFlags.HasFlag(ImWindowBehaviorFlags.AllowDocking))
        {
            var windowClass = new ImGuiWindowClass
            {
                ClassId = ImGui.GetID(GetImGuiId()),
                DockingAllowUnclassed = 1
            };
            unsafe { ImGui.SetNextWindowClass(ref windowClass); }
        }

        // ── Begin ──
        bool open = m_IsOpen;
        bool expanded = ImGui.Begin(GetImGuiId(), ref open, Flags);

        // ── 开关状态变化检测 ──
        if (open != m_IsOpen)
        {
            m_IsOpen = open;
            if (!open) OnClose();
        }

        if (expanded)
        {
            // ── 焦点跟踪 ──
            bool nowFocused = ImGui.IsWindowFocused(ImGuiFocusedFlags.RootAndChildWindows);
            if (nowFocused != m_WasFocused)
            {
                IsFocused = nowFocused;
                if (nowFocused) OnFocus();
                else OnLostFocus();
                m_WasFocused = nowFocused;
            }

            // ── 菜单栏 ──
            if (HasMenuBar() && ImGui.BeginMenuBar())
            {
                OnRenderMenuBar();
                ImGui.EndMenuBar();
            }

            // ── 内容 ──
            OnRender();
        }

        // ── End ──
        ImGui.End();
    }

    // ── 关闭 ──

    /// <inheritdoc />
    public void RequestClose()
    {
        m_IsOpen = false;
        OnClose();
    }

    // ── IDisposable ──

    /// <summary>释放资源并关闭窗口。</summary>
    public void Dispose()
    {
        if (!m_Disposed)
        {
            m_Disposed = true;
            OnClose();
        }
    }
}
