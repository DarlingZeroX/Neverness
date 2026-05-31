using System.Collections.Generic;

namespace NevernessLauncher.Core.Models
{
    /// <summary>
    /// 启动前检查结果模型
    /// </summary>
    public class PreLaunchCheckResult
    {
        /// <summary>是否可以启动</summary>
        public bool CanLaunch { get; set; } = true;

        /// <summary>问题列表</summary>
        public List<string> Issues { get; set; } = new();

        /// <summary>警告列表</summary>
        public List<string> Warnings { get; set; } = new();
    }
}
