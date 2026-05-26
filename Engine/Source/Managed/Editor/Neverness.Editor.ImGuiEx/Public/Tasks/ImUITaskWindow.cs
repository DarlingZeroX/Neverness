using Hexa.NET.ImGui;
using System.Numerics;

namespace Neverness.Editor.ImGuiEx;

/// <summary>
/// 任务进度窗口，显示所有活跃的后台任务进度。
///
/// 继承 ImWindow，支持与窗口管理器集成。
/// 渲染每个任务的：标签、进度条、状态文本、取消按钮。
/// </summary>
public sealed class ImUITaskWindow : ImWindow
{
    private readonly List<IImUITask> m_Tasks = new();
    private readonly object m_Lock = new();

    public ImUITaskWindow() : base("Background Tasks")
    {
        Flags = ImGuiWindowFlags.NoCollapse;
        BehaviorFlags = ImWindowBehaviorFlags.Default;
    }

    /// <summary>添加一个正在运行的任务到窗口跟踪列表。</summary>
    public void TrackTask(IImUITask task)
    {
        ArgumentNullException.ThrowIfNull(task);
        lock (m_Lock)
        {
            m_Tasks.Add(task);
        }
    }

    /// <summary>移除已完成/已取消的任务。</summary>
    public void PruneCompleted()
    {
        lock (m_Lock)
        {
            m_Tasks.RemoveAll(t =>
                t.State == ImTaskState.Completed ||
                t.State == ImTaskState.Cancelled);
        }
    }

    /// <summary>当前跟踪的任务数量。</summary>
    public int TaskCount
    {
        get { lock (m_Lock) { return m_Tasks.Count; } }
    }

    protected override void OnRender()
    {
        IImUITask[] snapshot;
        lock (m_Lock)
        {
            if (m_Tasks.Count == 0)
            {
                ImGui.TextDisabled("没有活跃的后台任务。");
                return;
            }
            snapshot = m_Tasks.ToArray();
        }

        bool hasCompleted = false;

        foreach (var task in snapshot)
        {
            DrawTaskItem(task);
            if (task.State == ImTaskState.Completed || task.State == ImTaskState.Cancelled)
                hasCompleted = true;
        }

        // 自动清理已完成的任务
        if (hasCompleted)
        {
            PruneCompleted();
        }
    }

    private static void DrawTaskItem(IImUITask task)
    {
        // 状态标记
        string statusIcon = task.State switch
        {
            ImTaskState.Running => "[~]",
            ImTaskState.Completed => "[v]",
            ImTaskState.Faulted => "[!]",
            ImTaskState.Cancelled => "[x]",
            _ => "",
        };

        ImGui.Text($"{statusIcon} {task.Label}");

        // 进度条
        if (task.State == ImTaskState.Running)
        {
            float progress = task.Progress < 0 ? 0 : task.Progress;
            bool indeterminate = task.Progress < 0;

            if (indeterminate)
            {
                // 不确定进度：显示脉冲动画条
                float pulse = (float)(Math.Sin(DateTime.UtcNow.TimeOfDay.TotalSeconds * 4) * 0.5 + 0.5);
                ImGui.ProgressBar(pulse, new Vector2(-1, 0), "");
            }
            else
            {
                ImGui.ProgressBar(progress, new Vector2(-1, 0), $"{progress:P0}");
            }

            // 取消按钮
            if (ImGui.SmallButton($"取消##{task.TaskId:N}"))
            {
                task.Cancel();
            }
        }
        else if (task.State == ImTaskState.Faulted)
        {
            ImGui.TextColored(new Vector4(0.9f, 0.2f, 0.2f, 1f), "任务失败");
        }

        // 状态文本
        if (!string.IsNullOrEmpty(task.StatusText))
        {
            ImGui.TextDisabled(task.StatusText);
        }

        ImGui.Separator();
    }
}
