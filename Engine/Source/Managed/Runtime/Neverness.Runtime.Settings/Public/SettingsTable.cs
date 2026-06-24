using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Neverness.Runtime.Settings;

/// <summary>
/// 所有设置表的基类。
/// 子类定义字段，系统自动处理序列化、UI 渲染、变更通知。
///
/// 用法：
/// <code>
/// [SettingTable("graphics", "图形", Category = "渲染")]
/// public sealed class GraphicsSettings : SettingsTable
/// {
///     public override string TableId => "graphics";
///     public override string DisplayName => "图形";
///
///     [SettingField(DisplayName = "垂直同步")]
///     public bool VSync { get; set; } = true;
/// }
/// </code>
/// </summary>
public abstract class SettingsTable : INotifyPropertyChanged
{
    // ── 抽象属性 ──

    /// <summary>设置表唯一 ID（如 "graphics"）。</summary>
    public abstract string TableId { get; }

    /// <summary>设置表显示名称（如 "图形"）。</summary>
    public abstract string DisplayName { get; }

    // ── 虚属性 ──

    /// <summary>设置范围（Project 或 User）。</summary>
    public virtual SettingsScope Scope => SettingsScope.Project;

    /// <summary>设置表图标（可选，用于 TreeView）。</summary>
    public virtual string? Icon => null;

    /// <summary>设置表分类（用于 TreeView 分组）。</summary>
    public virtual string? Category => null;

    // ── 序列化 ──

    /// <summary>从 JSON 字符串加载设置值。</summary>
    /// <param name="json">JSON 字符串。</param>
    public void LoadFromJson(string json)
    {
        if (string.IsNullOrWhiteSpace(json))
            return;

        try
        {
            var options = CreateJsonOptions();
            var data = JsonSerializer.Deserialize<Dictionary<string, JsonElement>>(json, options);
            if (data == null) return;

            var type = GetType();
            foreach (var kvp in data)
            {
                // 按 camelCase 属性名匹配
                var prop = type.GetProperty(kvp.Key,
                    System.Reflection.BindingFlags.Public |
                    System.Reflection.BindingFlags.Instance |
                    System.Reflection.BindingFlags.IgnoreCase);

                if (prop == null || !prop.CanWrite)
                    continue;

                try
                {
                    var value = kvp.Value.Deserialize(prop.PropertyType, options);
                    prop.SetValue(this, value);
                }
                catch
                {
                    // 字段类型不匹配，跳过（向前兼容）
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[SettingsTable] 加载 '{TableId}' 失败: {ex.Message}");
        }
    }

    /// <summary>序列化为 JSON 字符串。</summary>
    /// <returns>格式化的 JSON 字符串。</returns>
    public string SaveToJson()
    {
        try
        {
            var options = CreateJsonOptions();
            var type = GetType();
            var dict = new Dictionary<string, object?>();

            // 收集所有公共实例属性
            foreach (var prop in type.GetProperties(
                System.Reflection.BindingFlags.Public |
                System.Reflection.BindingFlags.Instance))
            {
                if (!prop.CanRead || prop.Name is nameof(TableId) or nameof(DisplayName) or nameof(Icon) or nameof(Category))
                    continue;

                // 跳过 [SettingHidden] 标记的字段（仍然序列化，但在 UI 中隐藏）
                // 注意：[SettingHidden] 只影响 UI，不影响序列化
                dict[ToCamelCase(prop.Name)] = prop.GetValue(this);
            }

            // 收集所有公共实例字段
            foreach (var field in type.GetFields(
                System.Reflection.BindingFlags.Public |
                System.Reflection.BindingFlags.Instance))
            {
                if (field.IsInitOnly)
                    continue;

                dict[ToCamelCase(field.Name)] = field.GetValue(this);
            }

            return JsonSerializer.Serialize(dict, options);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[SettingsTable] 序列化 '{TableId}' 失败: {ex.Message}");
            return "{}";
        }
    }

    /// <summary>重置为默认值。子类应重写此方法。</summary>
    public virtual void ResetDefaults()
    {
        // 默认实现：创建新实例，复制所有属性值
        var type = GetType();
        var defaultInstance = Activator.CreateInstance(type);
        if (defaultInstance == null) return;

        foreach (var prop in type.GetProperties(
            System.Reflection.BindingFlags.Public |
            System.Reflection.BindingFlags.Instance))
        {
            if (!prop.CanWrite) continue;
            if (prop.Name is nameof(TableId) or nameof(DisplayName) or nameof(Icon) or nameof(Category))
                continue;

            prop.SetValue(this, prop.GetValue(defaultInstance));
        }

        foreach (var field in type.GetFields(
            System.Reflection.BindingFlags.Public |
            System.Reflection.BindingFlags.Instance))
        {
            if (field.IsInitOnly) continue;
            field.SetValue(this, field.GetValue(defaultInstance));
        }

        // 通知所有属性已变更
        OnPropertyChanged(string.Empty);
    }

    // ── 变更通知 ──

    /// <summary>属性变更事件。</summary>
    public event PropertyChangedEventHandler? PropertyChanged;

    /// <summary>触发属性变更通知。</summary>
    /// <param name="propertyName">属性名。使用 CallerMemberName 自动填充。</param>
    protected void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
    }

    /// <summary>设置属性值并触发变更通知。</summary>
    /// <typeparam name="T">属性类型。</typeparam>
    /// <param name="storage">字段引用。</param>
    /// <param name="value">新值。</param>
    /// <param name="propertyName">属性名。</param>
    /// <returns>值是否发生变化。</returns>
    protected bool SetProperty<T>(ref T storage, T value, [CallerMemberName] string? propertyName = null)
    {
        if (EqualityComparer<T>.Default.Equals(storage, value))
            return false;

        storage = value;
        OnPropertyChanged(propertyName);
        return true;
    }

    // ── 内部工具 ──

    /// <summary>创建 JSON 序列化选项。</summary>
    private static JsonSerializerOptions CreateJsonOptions()
    {
        return new JsonSerializerOptions
        {
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
            WriteIndented = true,
            DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
            PropertyNameCaseInsensitive = true,
            ReadCommentHandling = JsonCommentHandling.Skip,
            AllowTrailingCommas = true,
            Converters = { new JsonStringEnumConverter(JsonNamingPolicy.CamelCase) }
        };
    }

    /// <summary>将 PascalCase 转为 camelCase。</summary>
    private static string ToCamelCase(string name)
    {
        if (string.IsNullOrEmpty(name) || char.IsLower(name[0]))
            return name;

        return char.ToLowerInvariant(name[0]) + name[1..];
    }
}
