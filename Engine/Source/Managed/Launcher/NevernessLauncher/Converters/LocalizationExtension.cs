using Avalonia.Data;
using Avalonia.Markup.Xaml;
using Avalonia.Markup.Xaml.MarkupExtensions;
using NevernessLauncher.Contracts;
using System;

namespace NevernessLauncher.Converters
{
    /// <summary>
    /// 本地化标记扩展
    /// 用法: {local:Localize Key}
    /// </summary>
    public class LocalizeExtension : MarkupExtension
    {
        public string Key { get; set; }

        public LocalizeExtension(string key)
        {
            Key = key;
        }

        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            var localizationService = Program.ServiceProvider?.GetService(typeof(ILocalizationService)) as ILocalizationService;
            if (localizationService == null)
            {
                return Key;
            }

            return new Binding
            {
                Source = localizationService,
                Path = $"[{Key}]",
                Mode = BindingMode.OneWay
            };
        }
    }
}
