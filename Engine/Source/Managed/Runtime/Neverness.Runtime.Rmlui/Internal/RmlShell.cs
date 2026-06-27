using RmlUiNet;
using RmlUiNet.Input;

namespace Neverness.Runtime.Rmlui;

/// <summary>
/// RmlUI Shell 工具类。
///
/// 提供默认字体加载和键盘快捷键处理。
/// 对应 C++ Shell.h/Shell.cpp
/// </summary>
public static class RmlShell
{
    #region 字体加载

    /// <summary>
    /// 默认字体配置。
    /// </summary>
    private record struct FontFace(string Filename, bool IsFallback);

    /// <summary>
    /// 加载默认字体。
    /// 包括 Lato 系列、Noto Emoji、微软雅黑。
    /// </summary>
    /// <param name="fontsPath">字体目录路径（VFS 路径）。</param>
    public static void LoadFonts(string fontsPath)
    {
        // 确保路径以 / 结尾
        if (!fontsPath.EndsWith("/"))
            fontsPath += "/";

        FontFace[] fontFaces = [
            new("LatoLatin-Regular.ttf", false),
            new("LatoLatin-Italic.ttf", false),
            new("LatoLatin-Bold.ttf", false),
            new("LatoLatin-BoldItalic.ttf", false),
            new("NotoEmoji-Regular.ttf", false),
            new("msyh.ttc", true),  // 微软雅黑作为 fallback
        ];

        foreach (var face in fontFaces)
        {
            Rml.LoadFontFace(fontsPath + face.Filename, face.IsFallback);
        }
    }

    /// <summary>
    /// 加载默认字体（使用默认路径）。
    /// 默认路径：{VFS根目录}/fonts/
    /// </summary>
    /// <param name="engineResourcePath">引擎资源 VFS 根路径。</param>
    public static void LoadDefaultFonts(string engineResourcePath)
    {
        LoadFonts(engineResourcePath + "fonts");
    }

    #endregion

    #region 键盘快捷键

    /// <summary>
    /// 处理键盘快捷键。
    ///
    /// 优先级快捷键（在 Context 处理前）：
    /// - F8: 切换调试器
    /// - Ctrl+0: 恢复原生 DPI 比例
    /// - Ctrl+1: 设置 DPI 比例为 1.0
    /// - Ctrl+-: 缩小 DPI 比例
    /// - Ctrl++: 放大 DPI 比例
    ///
    /// 低优先级快捷键（在 Context 未处理时）：
    /// - Ctrl+R: 重新加载所有样式表
    /// </summary>
    /// <param name="context">RmlUi Context。</param>
    /// <param name="key">按键标识。</param>
    /// <param name="keyModifier">按键修饰符。</param>
    /// <param name="nativeDpRatio">原生 DPI 比例。</param>
    /// <param name="priority">是否为优先级处理阶段。</param>
    /// <returns>如果事件应继续传播返回 true。</returns>
    public static bool ProcessKeyDownShortcuts(
        Context context,
        KeyIdentifier key,
        KeyModifier keyModifier,
        float nativeDpRatio,
        bool priority)
    {
        if (context == null)
            return true;

        // Result should return true to allow the event to propagate to the next handler.
        bool result = false;

        // This function is intended to be called twice by the backend, before and after submitting the key event to the context.
        // This way we can intercept shortcuts that should take priority over the context, and then handle any shortcuts of lower priority
        // if the context did not intercept it.
        if (priority)
        {
            // Priority shortcuts are handled before submitting the key to the context.

            // Toggle debugger and set dp-ratio using Ctrl +/-/0 keys.
            if (key == KeyIdentifier.KI_F8)
            {
                RmlUiNet.Debugger.SetVisible(!RmlUiNet.Debugger.IsVisible());
            }
            else if (key == KeyIdentifier.KI_0 && (keyModifier & KeyModifier.KM_CTRL) != 0)
            {
                // Ctrl+0: 恢复原生 DPI 比例
                // TODO: 需要在 RmlUi.Net 中暴露 SetDensityIndependentPixelRatio
                // context.SetDensityIndependentPixelRatio(nativeDpRatio);
            }
            else if (key == KeyIdentifier.KI_1 && (keyModifier & KeyModifier.KM_CTRL) != 0)
            {
                // Ctrl+1: 设置 DPI 比例为 1.0
                // context.SetDensityIndependentPixelRatio(1.0f);
            }
            else if ((key == KeyIdentifier.KI_OEM_MINUS || key == KeyIdentifier.KI_SUBTRACT) &&
                     (keyModifier & KeyModifier.KM_CTRL) != 0)
            {
                // Ctrl+-: 缩小 DPI 比例
                // float newDpRatio = Math.Max(context.GetDensityIndependentPixelRatio() / 1.2f, 0.5f);
                // context.SetDensityIndependentPixelRatio(newDpRatio);
            }
            else if ((key == KeyIdentifier.KI_OEM_PLUS || key == KeyIdentifier.KI_ADD) &&
                     (keyModifier & KeyModifier.KM_CTRL) != 0)
            {
                // Ctrl++: 放大 DPI 比例
                // float newDpRatio = Math.Min(context.GetDensityIndependentPixelRatio() * 1.2f, 2.5f);
                // context.SetDensityIndependentPixelRatio(newDpRatio);
            }
            else
            {
                // Propagate the key down event to the context.
                result = true;
            }
        }
        else
        {
            // We arrive here when no priority keys are detected and the key was not consumed by the context.
            // Check for shortcuts of lower priority.
            if (key == KeyIdentifier.KI_R && (keyModifier & KeyModifier.KM_CTRL) != 0)
            {
                // Ctrl+R: 重新加载所有样式表
                ReloadAllStyleSheets(context);
            }
            else
            {
                result = true;
            }
        }

        return result;
    }

    /// <summary>
    /// 重新加载所有 .rml 文档的样式表。
    /// </summary>
    private static void ReloadAllStyleSheets(Context context)
    {
        // TODO: 需要在 RmlUi.Net 中暴露 GetNumDocuments 和 GetDocument API
        // for (int i = 0; i < context.GetNumDocuments(); i++)
        // {
        //     var document = context.GetDocument(i);
        //     var src = document.GetSourceURL();
        //     if (src.EndsWith(".rml"))
        //     {
        //         document.ReloadStyleSheet();
        //     }
        // }
    }

    #endregion

    #region 调试快捷键

    /// <summary>
    /// 处理调试相关的键盘事件。
    /// 简化版本，只处理 F8 切换调试器。
    /// </summary>
    /// <param name="key">按键标识。</param>
    /// <returns>如果处理了快捷键返回 true。</returns>
    public static bool HandleDebugShortcut(KeyIdentifier key)
    {
        if (key == KeyIdentifier.KI_F8)
        {
            RmlUiNet.Debugger.SetVisible(!RmlUiNet.Debugger.IsVisible());
            return true;
        }
        return false;
    }

    #endregion
}
