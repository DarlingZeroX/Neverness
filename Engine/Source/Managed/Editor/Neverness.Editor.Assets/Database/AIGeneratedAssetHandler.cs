using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets;

/// <summary>
/// AI 生成资产处理器（Phase 8 框架 stub）。
///
/// 负责将 AI 生成的内容（纹理、模型、音频等）注册为正式资产，
/// 通过 ImportPipeline 完成标准导入流程。
///
/// 使用方法：
///   AIGeneratedAssetHandler.RegisterGenerator("Texture2D", MyTextureGenerator);
///   var result = AIGeneratedAssetHandler.HandleAIGenerated(
///       "Assets/AI/gen_hero.png", "Texture2D", rawPngBytes);
///
/// 设计：
///   - 框架接口已定义，实际 AI 集成待后续实现
///   - 支持注册自定义生成器（按类型名）
///   - 生成器回调返回 ImportResult，由 ImportPipeline 处理后续流程
/// </summary>
public static class AIGeneratedAssetHandler
{
    private static readonly Dictionary<string, Func<AIGenRequest, ImportResult>> s_generators
        = new(StringComparer.OrdinalIgnoreCase);

    private static readonly object s_lock = new();

    /// <summary>注册 AI 资产生成器。</summary>
    /// <param name="typeName">资产类型名（如 "Texture2D", "Mesh"）</param>
    /// <param name="generator">生成器回调</param>
    public static void RegisterGenerator(string typeName, Func<AIGenRequest, ImportResult> generator)
    {
        if (string.IsNullOrEmpty(typeName))
            throw new ArgumentNullException(nameof(typeName));
        if (generator == null)
            throw new ArgumentNullException(nameof(generator));

        lock (s_lock)
        {
            s_generators[typeName] = generator;
        }
    }

    /// <summary>取消注册 AI 资产生成器。</summary>
    public static bool UnregisterGenerator(string typeName)
    {
        lock (s_lock)
        {
            return s_generators.Remove(typeName);
        }
    }

    /// <summary>检查是否有指定类型的生成器。</summary>
    public static bool HasGenerator(string typeName)
    {
        lock (s_lock)
        {
            return s_generators.ContainsKey(typeName);
        }
    }

    /// <summary>处理 AI 生成的资产数据，写入磁盘并通过 ImportPipeline 导入。</summary>
    /// <param name="outputPath">输出路径（如 "Assets/AI/gen_xxx.png"）</param>
    /// <param name="typeName">资产类型名</param>
    /// <param name="data">AI 生成的原始数据</param>
    /// <param name="prompt">可选的 AI 提示词</param>
    /// <returns>导入结果</returns>
    public static ImportResult HandleAIGenerated(
        string outputPath,
        string typeName,
        byte[] data,
        string? prompt = null)
    {
        if (string.IsNullOrEmpty(outputPath))
            return ImportResult.Fail("输出路径不能为空");
        if (data == null || data.Length == 0)
            return ImportResult.Fail("AI 生成数据为空");

        Func<AIGenRequest, ImportResult>? generator;
        lock (s_lock)
        {
            s_generators.TryGetValue(typeName, out generator);
        }

        if (generator == null)
            return ImportResult.Fail($"未注册的 AI 生成器类型: {typeName}");

        /* 写入 AI 生成的数据到磁盘 */
        var dir = Path.GetDirectoryName(outputPath);
        if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
            Directory.CreateDirectory(dir);

        File.WriteAllBytes(outputPath, data);

        /* 调用生成器 */
        var request = new AIGenRequest
        {
            OutputPath = outputPath,
            TypeName = typeName,
            Data = data,
            Prompt = prompt
        };

        try
        {
            var result = generator(request);

            if (result.Success)
            {
                /* 通过 ImportPipeline 正式导入 */
                ImportPipeline.Import(new NPath(outputPath));
            }

            return result;
        }
        catch (Exception ex)
        {
            return ImportResult.Fail($"AI 生成器异常: {ex.Message}");
        }
    }

    /// <summary>获取所有已注册的生成器类型名。</summary>
    public static IReadOnlyList<string> GetRegisteredTypes()
    {
        lock (s_lock)
        {
            return s_generators.Keys.ToList();
        }
    }
}

/// <summary>AI 资产生成请求。</summary>
public sealed class AIGenRequest
{
    /// <summary>AI 提示词（可选）。</summary>
    public string? Prompt { get; init; }

    /// <summary>输出文件路径。</summary>
    public required string OutputPath { get; init; }

    /// <summary>资产类型名。</summary>
    public required string TypeName { get; init; }

    /// <summary>AI 生成的原始数据。</summary>
    public required byte[] Data { get; init; }

    /// <summary>自定义参数（供生成器使用）。</summary>
    public Dictionary<string, object> Parameters { get; init; } = new();
}
