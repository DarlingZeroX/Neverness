namespace NevernessLauncher.Core.Enums
{
    /// <summary>
    /// 项目状态枚举
    /// </summary>
    public enum ProjectStatus
    {
        /// <summary>项目有效，可正常打开</summary>
        Valid,
        /// <summary>关联引擎版本不可用</summary>
        EngineMissing,
        /// <summary>项目路径无效</summary>
        InvalidPath,
        /// <summary>项目配置文件损坏</summary>
        Corrupted,
        /// <summary>引擎版本不兼容</summary>
        VersionMismatch
    }
}
