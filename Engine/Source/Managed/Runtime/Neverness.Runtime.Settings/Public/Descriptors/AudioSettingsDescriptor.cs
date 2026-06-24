namespace Neverness.Runtime.Settings.Descriptors;

/// <summary>
/// 音频设置描述符——手写，Phase 2 由 Source Generator 替代。
/// </summary>
public sealed class AudioSettingsDescriptor : ISettingsDescriptor
{
    public static readonly AudioSettingsDescriptor Instance = new();

    public Type TableType => typeof(AudioSettings);
    public string TableId => "audio";
    public string DisplayName => "音频";
    public SettingsScope Scope => SettingsScope.Project;
    public string? Category => "音频";
    public string? Icon => null;

    public IReadOnlyList<FieldDescriptor> Fields => new[]
    {
        new FieldDescriptor
        {
            Name = "MasterVolume",
            DisplayName = "主音量",
            FieldType = FieldType.Float,
            ValueType = typeof(float),
            Min = 0f, Max = 1f, Step = 0.01f,
            Order = 0,
            Getter = t => ((AudioSettings)t).MasterVolume,
            Setter = (t, v) => ((AudioSettings)t).MasterVolume = (float)v!,
        },
        new FieldDescriptor
        {
            Name = "MusicVolume",
            DisplayName = "音乐音量",
            FieldType = FieldType.Float,
            ValueType = typeof(float),
            Min = 0f, Max = 1f, Step = 0.01f,
            Order = 1,
            Getter = t => ((AudioSettings)t).MusicVolume,
            Setter = (t, v) => ((AudioSettings)t).MusicVolume = (float)v!,
        },
        new FieldDescriptor
        {
            Name = "SFXVolume",
            DisplayName = "音效音量",
            FieldType = FieldType.Float,
            ValueType = typeof(float),
            Min = 0f, Max = 1f, Step = 0.01f,
            Order = 2,
            Getter = t => ((AudioSettings)t).SFXVolume,
            Setter = (t, v) => ((AudioSettings)t).SFXVolume = (float)v!,
        },
        new FieldDescriptor
        {
            Name = "VoiceVolume",
            DisplayName = "语音音量",
            FieldType = FieldType.Float,
            ValueType = typeof(float),
            Min = 0f, Max = 1f, Step = 0.01f,
            Order = 3,
            Getter = t => ((AudioSettings)t).VoiceVolume,
            Setter = (t, v) => ((AudioSettings)t).VoiceVolume = (float)v!,
        },
        new FieldDescriptor
        {
            Name = "Muted",
            DisplayName = "静音",
            FieldType = FieldType.Bool,
            ValueType = typeof(bool),
            Order = 4,
            Getter = t => ((AudioSettings)t).Muted,
            Setter = (t, v) => ((AudioSettings)t).Muted = (bool)v!,
        },
    };

    public SettingsTable CreateDefault() => new AudioSettings();
}
