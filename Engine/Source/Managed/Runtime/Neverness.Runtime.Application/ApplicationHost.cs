// Neverness.Runtime.Application — 应用生命周期管理（C# 驱动）。
// 整合 SDL3-CS、窗口管理、事件泵、VFS 初始化。
// ImGui Backend 通过 ImGuiBackendBridge 调用 C++ 封装。

using Neverness.Runtime.Application.Private;
using Neverness.Runtime.Engine;

namespace Neverness.Runtime.Application;

/// <summary>
/// Runtime Host 生命周期封装。
/// C# 直接调用 SDL3-CS，不再经过 C++ NNApplicationApi。
/// </summary>
public static unsafe class ApplicationHost
{
    private static bool s_initialized;
    private static string? s_projectRoot;
    private static string? s_editorRoot;

    /// <summary>是否已初始化。</summary>
    public static bool IsAvailable => s_initialized;

    /// <summary>项目根目录。</summary>
    public static string? ProjectRoot => s_projectRoot;

    /// <summary>编辑器根目录。</summary>
    public static string? EditorRoot => s_editorRoot;

    /// <summary>
    /// 初始化应用生命周期。
    /// 顺序：NativeApiBootstrap → 项目根目录发现 → VFS → SDL3。
    /// </summary>
    public static bool Initialize()
    {
        if (s_initialized)
        {
            return true;
        }

        // 1. Native API Bootstrap（加载 DLL，验证版本）
        if (!EngineNativeApiBootstrap.IsInstalled)
        {
            Console.Error.WriteLine("[ApplicationHost] EngineNativeApiBootstrap 未安装");
            return false;
        }

        // 2. 发现项目根目录
        s_editorRoot = DiscoverEditorRoot();
        s_projectRoot = DiscoverProjectRoot(s_editorRoot);
        if (string.IsNullOrEmpty(s_projectRoot))
        {
            Console.Error.WriteLine("[ApplicationHost] 未找到可用的项目目录");
            return false;
        }

        Console.WriteLine($"[ApplicationHost] 项目目录: {s_projectRoot}");
        Console.WriteLine($"[ApplicationHost] 编辑器目录: {s_editorRoot}");

        // 3. VFS 初始化（在 SDL 之前）
        if (!EditorVfsInitializer.InitializeVfs(s_projectRoot, s_editorRoot))
        {
            Console.Error.WriteLine("[ApplicationHost] VFS 初始化失败");
            return false;
        }

        // 4. SDL3 初始化
        if (!SdlApplicationHost.Initialize())
        {
            Console.Error.WriteLine("[ApplicationHost] SDL3 初始化失败");
            return false;
        }

        s_initialized = true;
        Console.WriteLine("[ApplicationHost] 初始化成功");
        return true;
    }

    /// <summary>
    /// 泵送事件；返回 false 表示应退出主循环。
    /// </summary>
    public static bool PumpEvents()
    {
        if (!s_initialized)
        {
            return false;
        }

        return SdlEventBridge.PumpEvents();
    }

    /// <summary>
    /// 关闭所有子系统。
    /// </summary>
    public static void Shutdown()
    {
        if (!s_initialized)
        {
            return;
        }

        // 关闭 ImGui Backend
        ImGuiBackendBridge.Shutdown();

        // 销毁所有渲染表面
        RenderSurfaceHost.DestroyAll();

        // 销毁所有窗口
        SdlWindowManager.DestroyAll();

        // 关闭 SDL3
        SdlApplicationHost.Shutdown();

        s_initialized = false;
        Console.WriteLine("[ApplicationHost] 已关闭");
    }

    /// <summary>
    /// 帧开始（ImGui NewFrame）。
    /// </summary>
    public static void BeginFrame()
    {
        if (!s_initialized)
        {
            return;
        }

        // 如果 ImGui Backend 已初始化，调用 NewFrame
        if (ImGuiBackendBridge.IsInitialized)
        {
            var primary = SdlWindowManager.GetPrimaryWindow();
            if (primary != null)
            {
                var (w, h) = primary.GetSize();
                ImGuiBackendBridge.NewFrame(w, h);
            }
        }
    }

    /// <summary>
    /// 帧结束（ImGui Render + SwapChain Present）。
    /// </summary>
    public static void EndFrame()
    {
        if (!s_initialized)
        {
            return;
        }

        // 如果 ImGui Backend 已初始化，调用 Render
        if (ImGuiBackendBridge.IsInitialized)
        {
            // 获取 Diligent 设备指针（当前从 C++ 端获取）
            if (EngineNativeApiBootstrap.IsInstalled)
            {
                ref readonly var api = ref EngineNativeApiCache.EngineApi;
                if (api.Diligent.GetPrimaryContext != null && api.Diligent.GetPrimarySwapChain != null)
                {
                    var context = (IntPtr)api.Diligent.GetPrimaryContext();
                    var swapChain = (IntPtr)api.Diligent.GetPrimarySwapChain();
                    ImGuiBackendBridge.Render(context, swapChain);
                }

                // Present SwapChain
                if (api.Diligent.PresentPrimarySwapChain != null)
                {
                    api.Diligent.PresentPrimarySwapChain();
                }
            }
        }
    }

    /// <summary>
    /// 发现编辑器根目录（编译时定义或运行时查找）。
    /// </summary>
    private static string DiscoverEditorRoot()
    {
        // 编译时定义的项目根目录（C++ EDITOR_PROJECT_ROOT_DIR 等价）
        // 在 C# 中通过环境变量或配置文件获取
        var editorRoot = Environment.GetEnvironmentVariable("NN_EDITOR_ROOT");
        if (!string.IsNullOrEmpty(editorRoot) && Directory.Exists(editorRoot))
        {
            return editorRoot;
        }

        // 从当前工作目录向上查找
        var dir = Directory.GetCurrentDirectory();
        while (dir != null)
        {
            // 查找标记文件（如 CMakeLists.txt 或 Engine 目录）
            if (Directory.Exists(Path.Combine(dir, "Engine", "Source")))
            {
                return dir;
            }

            dir = Directory.GetParent(dir)?.FullName;
        }

        return Directory.GetCurrentDirectory();
    }

    /// <summary>
    /// 发现项目根目录。
    /// </summary>
    private static string? DiscoverProjectRoot(string editorRoot)
    {
        // 1. 尝试从 Data/EditorStartupData.txt 读取
        var startupDataPath = Path.Combine(editorRoot, "Data", "EditorStartupData.txt");
        if (File.Exists(startupDataPath))
        {
            try
            {
                var content = File.ReadAllText(startupDataPath).Trim();
                if (!string.IsNullOrEmpty(content) && Directory.Exists(content))
                {
                    return content;
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"[ApplicationHost] 读取 EditorStartupData.txt 失败: {ex.Message}");
            }
        }

        // 2. 尝试默认候选目录
        var candidates = new[]
        {
            Path.Combine(editorRoot, "Project", "示例项目"),
            Path.Combine(editorRoot, "Project", "Test Project"),
            Path.Combine(editorRoot, "Projects", "Test Project"),
        };

        foreach (var candidate in candidates)
        {
            if (LooksLikeProjectRoot(candidate))
            {
                return candidate;
            }
        }

        // 3. 遍历 Project 目录
        var projectDir = Path.Combine(editorRoot, "Project");
        if (Directory.Exists(projectDir))
        {
            foreach (var subDir in Directory.GetDirectories(projectDir))
            {
                if (LooksLikeProjectRoot(subDir))
                {
                    return subDir;
                }
            }
        }

        return null;
    }

    /// <summary>检查是否像项目根目录。</summary>
    private static bool LooksLikeProjectRoot(string path)
    {
        return Directory.Exists(path) &&
               Directory.Exists(Path.Combine(path, "Assets")) &&
               Directory.Exists(Path.Combine(path, "ProjectSettings")) &&
               Directory.Exists(Path.Combine(path, "Intermediate"));
    }
}
