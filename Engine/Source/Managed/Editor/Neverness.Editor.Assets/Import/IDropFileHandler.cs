namespace Neverness.Editor.Assets;

/// <summary>
/// 拖放文件导入处理器——每个实现声明自己支持的文件扩展名。
/// 通过 <see cref="DropFileHandlerAttribute"/> 标注实现类，<see cref="DropImportService"/> 反射自动发现。
/// </summary>
public interface IDropFileHandler
{
    /// <summary>是否能处理该文件路径。</summary>
    bool CanHandle(string filePath);

    /// <summary>
    /// 将外部文件导入到 Assets 目录：复制文件 + 生成 .meta + 注册到数据库。
    /// 返回 true 表示成功。
    /// </summary>
    bool Handle(string filePath, string assetsRoot);
}
