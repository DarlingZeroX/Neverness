using System;
using System.Collections.Generic;

namespace NevernessLauncher.Core.Models
{
    /// <summary>
    /// 最近项目缓存模型
    /// </summary>
    public class RecentProjectsCache
    {
        /// <summary>缓存版本</summary>
        public int Version { get; set; } = 1;

        /// <summary>最近项目列表</summary>
        public List<RecentProject> Projects { get; set; } = new();
    }
}
