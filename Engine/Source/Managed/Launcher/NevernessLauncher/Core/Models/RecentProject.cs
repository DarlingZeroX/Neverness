using NevernessLauncher.Core.Enums;
using System;

namespace NevernessLauncher.Core.Models
{
    /// <summary>
    /// 最近项目模型
    /// </summary>
    public class RecentProject
    {
        /// <summary>项目路径</summary>
        public string ProjectPath { get; set; } = string.Empty;

        /// <summary>项目名称</summary>
        public string ProjectName { get; set; } = string.Empty;

        /// <summary>最后访问时间</summary>
        public DateTimeOffset LastAccessTime { get; set; } = DateTimeOffset.Now;

        /// <summary>访问次数</summary>
        public int AccessCount { get; set; } = 1;

        /// <summary>是否置顶</summary>
        public bool IsPinned { get; set; }

        /// <summary>项目是否仍然有效</summary>
        public bool IsValid { get; set; } = true;

        /// <summary>关联引擎版本</summary>
        public string? EngineVersion { get; set; }

        /// <summary>项目状态</summary>
        public ProjectStatus ProjectStatus { get; set; } = ProjectStatus.Valid;
    }
}
