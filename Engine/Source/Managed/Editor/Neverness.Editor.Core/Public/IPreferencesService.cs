namespace Neverness.Editor.Core.Public;

/// <summary>
/// 编辑器偏好设置服务接口——管理用户可配置的编辑器偏好。
/// </summary>
public interface IPreferencesService
{
    /// <summary>获取或设置首选 IDE。</summary>
    IDEPreference PreferredIDE { get; set; }

    /// <summary>保存设置到持久化存储。</summary>
    void Save();

    /// <summary>从持久化存储加载设置。</summary>
    void Load();
}
