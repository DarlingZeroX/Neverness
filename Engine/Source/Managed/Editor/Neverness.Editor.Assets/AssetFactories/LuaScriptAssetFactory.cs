using Neverness.Editor.Framework.Public;

namespace Neverness.Editor.Assets.AssetFactories;

/// <summary>
/// Lua 脚本资产工厂——在指定目录创建空 Lua 脚本文件。
/// </summary>
public sealed class LuaScriptAssetFactory : IAssetFactory
{
    public string DisplayName => "Lua Script";
    public string Category => "Scripting";
    public string Icon => FontAwesome5Pro.Code;
    public string FileExtension => ".lua";

    public bool CreateAsset(string directoryPath)
    {
        try
        {
            var filePath = System.IO.Path.Combine(directoryPath, "New Script.lua");
            int index = 1;
            while (File.Exists(filePath))
            {
                filePath = System.IO.Path.Combine(directoryPath, $"New Script {index++}.lua");
            }

            var scriptName = System.IO.Path.GetFileNameWithoutExtension(filePath);

            var content = $$"""
                --- {{scriptName}}
                -- Auto-generated Lua script

                local M = {}

                function M.onInit()
                end

                function M.onUpdate(dt)
                end

                return M
                """;

            File.WriteAllText(filePath, content);
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
            return false;
        }
    }
}
