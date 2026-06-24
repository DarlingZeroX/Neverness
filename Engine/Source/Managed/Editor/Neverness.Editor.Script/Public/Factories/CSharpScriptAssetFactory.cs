// ============================================================================
// CSharpScriptAssetFactory.cs - C# 脚本资产工厂
// ============================================================================
// 在 ContentBrowser 右键菜单中创建 C# 脚本文件。
// 模板基于 EntityBehaviour 基类。
// ============================================================================

using Neverness.Editor.Assets.AssetFactories;
using Neverness.Runtime.Assets;
using Neverness.Runtime.VFS;

namespace Neverness.Editor.Script.Public;

/// <summary>
/// C# 脚本资产工厂——在指定目录创建 C# 脚本文件。
/// </summary>
public sealed class CSharpScriptAssetFactory : IAssetFactory
{
    // ========================================================================
    // IAssetFactory
    // ========================================================================

    /// <inheritdoc/>
    public string DisplayName => "C# Script";

    /// <inheritdoc/>
    public string Category => "Scripting";

    /// <inheritdoc/>
    public string Icon => "📝";

    /// <inheritdoc/>
    public string FileExtension => ".cs";

    /// <inheritdoc/>
    public NPath? CreateAsset(NPath directoryPath)
    {
        try
        {
            // 1. 生成唯一文件名
            var filePath = directoryPath.Combine("NewScript.cs");
            int index = 1;
            while (File.Exists(filePath.FullPath))
            {
                filePath = directoryPath.Combine($"NewScript{index++}.cs");
            }

            var scriptName = filePath.FileNameWithoutExtension;

            // 2. 生成脚本内容
            var content = GenerateScriptContent(scriptName);

            // 3. 写入文件
            File.WriteAllText(filePath.FullPath, content);

            return filePath;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[CSharpScriptAssetFactory] Failed to create script: {ex.Message}");
            return null;
        }
    }

    // ========================================================================
    // 内部方法
    // ========================================================================

    /// <summary>生成 C# 脚本内容。</summary>
    private static string GenerateScriptContent(string scriptName)
    {
        // 确保脚本名是有效的 C# 标识符
        scriptName = SanitizeIdentifier(scriptName);

        return $$"""
            // ============================================================================
            // {{scriptName}}.cs
            // ============================================================================
            // Auto-generated C# script using Neverness Gameplay Framework.
            // ============================================================================

            using Neverness.Gameplay;

            namespace Game.Scripts
            {
                /// <summary>
                /// {{scriptName}} 脚本。
                /// </summary>
                [AutoRegisterScript]
                public class {{scriptName}} : EntityBehaviour
                {
                    // ========================================================================
                    // 公共字段（可在 Inspector 中编辑）
                    // ========================================================================

                    /// <summary>移动速度。</summary>
                    public float Speed = 5.0f;

                    // ========================================================================
                    // 生命周期回调
                    // ========================================================================

                    /// <summary>
                    /// 组件创建时调用（Awake）。
                    /// ⚠️ 禁止在此访问其他 Entity。
                    /// </summary>
                    public override void OnCreate()
                    {
                        base.OnCreate();
                        Debug.Log("{{scriptName}}: OnCreate");
                    }

                    /// <summary>
                    /// 首次 Update 前调用（Start）。
                    /// 可以安全访问其他 Entity。
                    /// </summary>
                    public override void OnStart()
                    {
                        base.OnStart();
                        Debug.Log("{{scriptName}}: OnStart");
                    }

                    /// <summary>
                    /// 每帧调用。
                    /// </summary>
                    /// <param name="deltaTime">帧间隔时间（秒）。</param>
                    public override void OnUpdate(float deltaTime)
                    {
                        base.OnUpdate(deltaTime);

                        // TODO: 在此添加游戏逻辑
                    }

                    /// <summary>
                    /// 固定时间步调用（适用于物理模拟）。
                    /// </summary>
                    /// <param name="fixedDeltaTime">固定时间步（秒）。</param>
                    public override void OnFixedUpdate(float fixedDeltaTime)
                    {
                        base.OnFixedUpdate(fixedDeltaTime);

                        // TODO: 在此添加物理相关逻辑
                    }

                    /// <summary>
                    /// 每帧末尾调用（适用于相机跟随等后处理）。
                    /// </summary>
                    /// <param name="deltaTime">帧间隔时间（秒）。</param>
                    public override void OnLateUpdate(float deltaTime)
                    {
                        base.OnLateUpdate(deltaTime);

                        // TODO: 在此后处理逻辑
                    }

                    /// <summary>
                    /// 组件销毁时调用。
                    /// </summary>
                    public override void OnDestroy()
                    {
                        Debug.Log("{{scriptName}}: OnDestroy");
                        base.OnDestroy();
                    }
                }
            }
            """;
    }

    /// <summary>清理标识符，确保是有效的 C# 标识符。</summary>
    private static string SanitizeIdentifier(string name)
    {
        if (string.IsNullOrWhiteSpace(name))
            return "NewScript";

        // 移除无效字符
        var sb = new System.Text.StringBuilder();
        for (int i = 0; i < name.Length; i++)
        {
            char c = name[i];
            if (i == 0)
            {
                // 首字符必须是字母或下划线
                if (char.IsLetter(c) || c == '_')
                    sb.Append(c);
                else
                    sb.Append('_');
            }
            else
            {
                // 后续字符可以是字母、数字或下划线
                if (char.IsLetterOrDigit(c) || c == '_')
                    sb.Append(c);
            }
        }

        var result = sb.ToString();

        // 确保不为空
        if (string.IsNullOrEmpty(result))
            return "NewScript";

        // 确保不是 C# 关键字
        var keywords = new HashSet<string>
        {
            "abstract", "as", "base", "bool", "break", "byte", "case", "catch", "char", "checked",
            "class", "const", "continue", "decimal", "default", "delegate", "do", "double", "else",
            "enum", "event", "explicit", "extern", "false", "finally", "fixed", "float", "for",
            "foreach", "goto", "if", "implicit", "in", "int", "interface", "internal", "is", "lock",
            "long", "namespace", "new", "null", "object", "operator", "out", "override", "params",
            "private", "protected", "public", "readonly", "ref", "return", "sbyte", "sealed", "short",
            "sizeof", "stackalloc", "static", "string", "struct", "switch", "this", "throw", "true",
            "try", "typeof", "uint", "ulong", "unchecked", "unsafe", "ushort", "using", "virtual",
            "void", "volatile", "while"
        };

        if (keywords.Contains(result))
            return "_" + result;

        return result;
    }
}
