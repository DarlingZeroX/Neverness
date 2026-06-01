// ============================================================================
// IInputProvider.cs - 输入提供者接口
// ============================================================================
// 输入提供者接口，避免 static 全局污染，支持多后端。
// ============================================================================

namespace Neverness.Gameplay;

/// <summary>
/// 输入提供者接口：避免 static 全局污染，支持多后端。
/// </summary>
public interface IInputProvider
{
    // ========================================================================
    // 键盘
    // ========================================================================

    /// <summary>检查按键是否按下。</summary>
    bool GetKey(KeyCode key);

    /// <summary>检查按键是否刚刚按下。</summary>
    bool GetKeyDown(KeyCode key);

    /// <summary>检查按键是否刚刚释放。</summary>
    bool GetKeyUp(KeyCode key);

    // ========================================================================
    // 鼠标
    // ========================================================================

    /// <summary>检查鼠标按键是否按下。</summary>
    bool GetMouseButton(MouseButton button);

    /// <summary>检查鼠标按键是否刚刚按下。</summary>
    bool GetMouseButtonDown(MouseButton button);

    /// <summary>检查鼠标按键是否刚刚释放。</summary>
    bool GetMouseButtonUp(MouseButton button);

    /// <summary>鼠标位置（屏幕坐标）。</summary>
    Vector2 MousePosition { get; }

    /// <summary>鼠标滚轮增量。</summary>
    float MouseScrollDelta { get; }

    // ========================================================================
    // 轴
    // ========================================================================

    /// <summary>获取轴值（-1 到 1，带平滑）。</summary>
    float GetAxis(string axisName);

    /// <summary>获取原始轴值（无平滑）。</summary>
    float GetAxisRaw(string axisName);
}
