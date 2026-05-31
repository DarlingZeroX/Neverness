using NevernessLauncher.Core.Enums;
using System;
using System.Collections.Generic;

namespace NevernessLauncher.Core.Models
{
    /// <summary>
    /// 项目信息模型
    /// </summary>
    public class ProjectInfo
    {
        /// <summary>项目显示名称</summary>
        public string Name { get; set; } = string.Empty;

        /// <summary>项目唯一标识</summary>
        public Guid Id { get; set; } = Guid.NewGuid();

        /// <summary>项目描述</summary>
        public string Description { get; set; } = string.Empty;

        /// <summary>作者</summary>
        public string Author { get; set; } = string.Empty;

        /// <summary>项目根目录路径</summary>
        public string ProjectPath { get; set; } = string.Empty;

        /// <summary>.nnproject 文件路径</summary>
        public string ProjectFilePath { get; set; } = string.Empty;

        /// <summary>绑定的引擎版本号</summary>
        public string EngineVersion { get; set; } = string.Empty;

        /// <summary>引擎渠道</summary>
        public EngineChannel EngineChannel { get; set; } = EngineChannel.Stable;

        /// <summary>最低引擎版本要求</summary>
        public string MinimumEngineVersion { get; set; } = string.Empty;

        /// <summary>创建时间</summary>
        public DateTimeOffset CreatedTime { get; set; } = DateTimeOffset.Now;

        /// <summary>最后修改时间</summary>
        public DateTimeOffset ModifiedTime { get; set; } = DateTimeOffset.Now;

        /// <summary>最后打开时间</summary>
        public DateTimeOffset? LastOpenedTime { get; set; }

        /// <summary>默认场景路径</summary>
        public string? DefaultScene { get; set; }

        /// <summary>目标平台</summary>
        public string TargetPlatform { get; set; } = "Windows";

        /// <summary>项目状态</summary>
        public ProjectStatus Status { get; set; } = ProjectStatus.Valid;

        /// <summary>资产目录</summary>
        public string AssetsPath { get; set; } = "Assets";

        /// <summary>场景目录</summary>
        public string ScenesPath { get; set; } = "Scenes";

        /// <summary>配置目录</summary>
        public string ConfigPath { get; set; } = "Config";

        /// <summary>构建输出目录</summary>
        public string BuildPath { get; set; } = "Build";
    }
}
