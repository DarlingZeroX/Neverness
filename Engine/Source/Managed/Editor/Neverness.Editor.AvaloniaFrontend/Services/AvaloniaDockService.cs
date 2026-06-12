using Neverness.Editor.Framework.Public.Services;

namespace Neverness.Editor.AvaloniaFrontend.Services;

/// <summary>
/// Avalonia Dock 服务实现——管理编辑器面板布局。
///
/// 基于 wieslawsoltes/Dock 库实现。
/// </summary>
public class AvaloniaDockService : IDockService
{
    /// <summary>默认布局文件路径。</summary>
    private static readonly string DefaultLayoutPath =
        Path.Combine(AppContext.BaseDirectory, "EditorLayout.json");

    public bool SaveLayout(string filePath)
    {
        try
        {
            // TODO: Phase 1 实现 - 使用 Dock 序列化器保存布局
            Console.WriteLine($"[AvaloniaDockService] 保存布局到: {filePath}");
            return true;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[AvaloniaDockService] 保存布局失败: {ex.Message}");
            return false;
        }
    }

    public bool LoadLayout(string filePath)
    {
        try
        {
            // TODO: Phase 1 实现 - 使用 Dock 反序列化器恢复布局
            Console.WriteLine($"[AvaloniaDockService] 从文件恢复布局: {filePath}");
            return true;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine($"[AvaloniaDockService] 恢复布局失败: {ex.Message}");
            return false;
        }
    }

    public void ResetToDefault()
    {
        // TODO: Phase 1 实现 - 重置为默认布局
        Console.WriteLine("[AvaloniaDockService] 重置为默认布局");
    }

    public void AddPanel(string panelId, DockArea area)
    {
        // TODO: Phase 1 实现 - 添加面板到指定区域
        Console.WriteLine($"[AvaloniaDockService] 添加面板: {panelId} -> {area}");
    }

    public void RemovePanel(string panelId)
    {
        // TODO: Phase 1 实现 - 移除面板
        Console.WriteLine($"[AvaloniaDockService] 移除面板: {panelId}");
    }

    public bool IsPanelDocked(string panelId)
    {
        // TODO: Phase 1 实现
        return false;
    }

    public void SetPanelVisible(string panelId, bool visible)
    {
        // TODO: Phase 1 实现
        Console.WriteLine($"[AvaloniaDockService] 面板可见性: {panelId} = {visible}");
    }
}
