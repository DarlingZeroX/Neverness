using Neverness.Editor.Framework.Public;
using Neverness.Runtime.Assets;

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
