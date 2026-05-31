namespace NevernessLauncher.Core.Enums
{
    /// <summary>
    /// 引擎发现来源枚举
    /// </summary>
    public enum EngineSource
    {
        /// <summary>从注册表发现</summary>
        Registry,
        /// <summary>从环境变量发现</summary>
        EnvironmentVariable,
        /// <summary>从固定路径发现</summary>
        FixedPath,
        /// <summary>用户手动指定</summary>
        UserDefined,
        /// <summary>从 Launcher 相对路径发现</summary>
        RelativePath
    }
}
