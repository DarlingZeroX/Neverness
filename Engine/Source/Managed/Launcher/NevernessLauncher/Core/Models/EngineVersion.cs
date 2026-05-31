using NevernessLauncher.Core.Enums;
using System;

namespace NevernessLauncher.Core.Models
{
    /// <summary>
    /// 引擎版本模型
    /// </summary>
    public class EngineVersion
    {
        /// <summary>引擎版本号</summary>
        public Version Version { get; set; } = new Version(0, 0, 0);

        /// <summary>渠道</summary>
        public EngineChannel Channel { get; set; } = EngineChannel.Stable;

        /// <summary>引擎安装路径</summary>
        public string InstallPath { get; set; } = string.Empty;

        /// <summary>Editor 可执行文件路径</summary>
        public string EditorPath { get; set; } = string.Empty;

        /// <summary>Runtime 可执行文件路径</summary>
        public string RuntimePath { get; set; } = string.Empty;

        /// <summary>是否为默认引擎版本</summary>
        public bool IsDefault { get; set; }

        /// <summary>引擎安装是否完整有效</summary>
        public bool IsValid { get; set; }

        /// <summary>发现时间</summary>
        public DateTimeOffset DiscoveredTime { get; set; } = DateTimeOffset.Now;

        /// <summary>发现来源</summary>
        public EngineSource Source { get; set; } = EngineSource.FixedPath;

        /// <summary>版本显示名称</summary>
        public string DisplayName => $"{Version}-{Channel}";
    }
}
