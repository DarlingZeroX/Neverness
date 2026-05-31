using System;
using System.Collections.Generic;

namespace NevernessLauncher.Contracts
{
    /// <summary>
    /// 本地化服务接口
    /// </summary>
    public interface ILocalizationService
    {
        /// <summary>当前语言</summary>
        string CurrentLanguage { get; }

        /// <summary>可用语言列表</summary>
        IReadOnlyList<LanguageInfo> AvailableLanguages { get; }

        /// <summary>获取本地化字符串</summary>
        string GetString(string key);

        /// <summary>获取本地化字符串（带格式化参数）</summary>
        string GetString(string key, params object[] args);

        /// <summary>切换语言</summary>
        void SetLanguage(string languageCode);

        /// <summary>语言变更事件</summary>
        event EventHandler<string>? LanguageChanged;
    }

    /// <summary>
    /// 语言信息
    /// </summary>
    public class LanguageInfo
    {
        /// <summary>语言代码 (如 zh-CN, en-US)</summary>
        public string Code { get; set; } = string.Empty;

        /// <summary>显示名称 (如 中文, English)</summary>
        public string DisplayName { get; set; } = string.Empty;

        /// <summary>本地名称 (如 中文, English)</summary>
        public string NativeName { get; set; } = string.Empty;
    }
}
