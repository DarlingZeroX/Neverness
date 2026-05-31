using NevernessLauncher.Core.Models;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace NevernessLauncher.Contracts
{
    /// <summary>
    /// 引擎服务接口
    /// </summary>
    public interface IEngineService
    {
        /// <summary>扫描本地引擎安装</summary>
        Task<IReadOnlyList<EngineVersion>> ScanEngines();

        /// <summary>获取已安装的引擎列表</summary>
        IReadOnlyList<EngineVersion> GetInstalledEngines();

        /// <summary>查找指定版本的引擎</summary>
        EngineVersion? FindEngine(Version version);

        /// <summary>查找兼容的引擎版本（最低版本要求）</summary>
        EngineVersion? FindCompatibleEngine(Version minimumVersion);

        /// <summary>获取默认引擎版本</summary>
        EngineVersion? GetDefaultEngine();

        /// <summary>设置默认引擎版本</summary>
        Task SetDefaultEngine(Version version);

        /// <summary>验证引擎安装是否有效</summary>
        bool ValidateEngine(EngineVersion engine);

        /// <summary>添加自定义引擎扫描路径</summary>
        Task AddEnginePath(string path);

        /// <summary>移除自定义引擎扫描路径</summary>
        Task RemoveEnginePath(string path);

        /// <summary>获取所有引擎扫描路径</summary>
        IReadOnlyList<string> GetEnginePaths();

        /// <summary>引擎列表变更事件</summary>
        event EventHandler? EnginesChanged;
    }
}
