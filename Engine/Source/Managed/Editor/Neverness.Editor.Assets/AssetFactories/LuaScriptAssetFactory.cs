using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Assets;

namespace Neverness.Editor.Assets.AssetFactories;

/// <summary>
/// Lua 脚本资产工厂——在指定目录创建 Lua 脚本文件。
/// 使用 EditorResourceCache 中的模板内容。
/// </summary>
public sealed class LuaScriptAssetFactory : IAssetFactory
{
    public string DisplayName => "Lua Script";
    public string Category => "Scripting";
    public string Icon => "📜";
    public string FileExtension => ".lua";

    public NPath? CreateAsset(NPath directoryPath)
    {
        try
        {
            var filePath = directoryPath.Combine("New Script.lua");
            int index = 1;
            while (File.Exists(filePath.FullPath))
            {
                filePath = directoryPath.Combine($"New Script {index++}.lua");
            }

            var scriptName = filePath.FileNameWithoutExtension;

            // 从 EditorResourceCache 获取模板内容
            var content = EditorResourceCache.Instance.GetLuaScriptTemplate(scriptName);

            // 模板加载失败时使用硬编码兜底内容
            if (string.IsNullOrEmpty(content))
            {
                content = $$"""
                    --- {{scriptName}}
                    -- Auto-generated Lua script

                    local M = {}

                    function M.onInit()
                    end

                    function M.onUpdate(dt)
                    end

                    return M
                    """;
            }

            File.WriteAllText(filePath.FullPath, content);
            return filePath;
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
            return null;
        }
    }
}
