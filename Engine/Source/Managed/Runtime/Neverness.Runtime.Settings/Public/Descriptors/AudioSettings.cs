using Neverness.Runtime.Settings.Attributes;

namespace Neverness.Runtime.Settings.Descriptors;

/// <summary>
/// 音频设置——运行时 Project Settings。
/// </summary>
[SettingTable("audio", "音频", Scope = SettingsScope.Project, Category = "音频")]
public sealed class AudioSettings : SettingsTable
{
    public override string TableId => "audio";
    public override string DisplayName => "音频";
    public override SettingsScope Scope => SettingsScope.Project;
    public override string? Category => "音频";

    [SettingField(DisplayName = "主音量")]
    [SettingRange(0f, 1f)]
    public float MasterVolume { get; set; } = 1.0f;

    [SettingField(DisplayName = "音乐音量")]
    [SettingRange(0f, 1f)]
    public float MusicVolume { get; set; } = 0.8f;

    [SettingField(DisplayName = "音效音量")]
    [SettingRange(0f, 1f)]
    public float SFXVolume { get; set; } = 1.0f;

    [SettingField(DisplayName = "语音音量")]
    [SettingRange(0f, 1f)]
    public float VoiceVolume { get; set; } = 1.0f;

    [SettingField(DisplayName = "静音")]
    public bool Muted { get; set; } = false;
}
