// ============================================================================
// Input.cs - 输入系统静态 API
// ============================================================================
// 输入系统静态 API，通过 GameplayContext 获取 IInputProvider。
// IInputProvider/KeyCode/MouseButton 已下沉到 Neverness.Runtime.Application。
// ============================================================================

using Neverness.Runtime.Application;

namespace Neverness.Gameplay;

/// <summary>
/// 输入系统静态 API。
/// </summary>
/// <remarks>
/// 通过 GameplayContext 获取 IInputProvider，避免 static 全局污染。
/// </remarks>
public static class Input
{
    // ========================================================================
    // 内部状态
    // ========================================================================

    /// <summary>输入提供者。</summary>
    public static IInputProvider? _provider;

    // ========================================================================
    // 初始化
    // ========================================================================

    /// <summary>设置输入提供者（由引擎初始化时调用）。</summary>
    /// <param name="provider">输入提供者。</param>
    internal static void SetProvider(IInputProvider provider)
    {
        _provider = provider;
    }

    // ========================================================================
    // 键盘
    // ========================================================================

    /// <summary>检查按键是否按下。</summary>
    public static bool GetKey(KeyCode key) => _provider?.GetKey(key) ?? false;

    /// <summary>检查按键是否刚刚按下。</summary>
    public static bool GetKeyDown(KeyCode key) => _provider?.GetKeyDown(key) ?? false;

    /// <summary>检查按键是否刚刚释放。</summary>
    public static bool GetKeyUp(KeyCode key) => _provider?.GetKeyUp(key) ?? false;

    // ========================================================================
    // 鼠标
    // ========================================================================

    /// <summary>检查鼠标按键是否按下。</summary>
    public static bool GetMouseButton(MouseButton button) => _provider?.GetMouseButton(button) ?? false;

    /// <summary>检查鼠标按键是否刚刚按下。</summary>
    public static bool GetMouseButtonDown(MouseButton button) => _provider?.GetMouseButtonDown(button) ?? false;

    /// <summary>检查鼠标按键是否刚刚释放。</summary>
    public static bool GetMouseButtonUp(MouseButton button) => _provider?.GetMouseButtonUp(button) ?? false;

    /// <summary>鼠标位置（屏幕坐标）。</summary>
    public static (float X, float Y) MousePosition => _provider?.MousePosition ?? (0f, 0f);

    /// <summary>鼠标滚轮增量。</summary>
    public static float MouseScrollDelta => _provider?.MouseScrollDelta ?? 0f;

    // ========================================================================
    // 轴
    // ========================================================================

    /// <summary>获取轴值（-1 到 1，带平滑）。</summary>
    public static float GetAxis(string axisName) => _provider?.GetAxis(axisName) ?? 0f;

    /// <summary>获取原始轴值（无平滑）。</summary>
    public static float GetAxisRaw(string axisName) => _provider?.GetAxisRaw(axisName) ?? 0f;
}
