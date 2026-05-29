namespace Neverness.Editor.MediaImporter;

/// <summary>
/// 媒体导入器模块入口。
///
/// ImporterRegistry 通过反射自动发现本程序集中的 IAssetImporter 实现
///（AudioImporter、VideoAssetImporter），无需手动注册。
///
/// 此 Install() 方法仅确保程序集在 ImporterRegistry.Discover() 之前被加载。
/// .NET 按需加载程序集，如果没有任何类型被引用，程序集不会自动加载。
/// </summary>
public static class MediaImporterModule
{
    public static void Install()
    {
        // ImporterRegistry.Discover() 会扫描所有 Neverness.Editor.* 程序集
        // 本方法确保 Neverness.Editor.MediaImporter 程序集在此调用之前已被加载到 AppDomain
    }
}
