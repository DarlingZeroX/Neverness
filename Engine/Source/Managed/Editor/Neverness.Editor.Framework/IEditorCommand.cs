namespace VisionGal.Managed.Editor;

/// <summary>
/// 可註冊至 <see cref="CommandRegistry"/> 之編輯器命令契約。
/// </summary>
public interface IEditorCommand
{
	/// <summary>命令唯一識別碼（如 <c>file.save</c>）。</summary>
	string CommandId { get; }

	/// <summary>顯示名稱。</summary>
	string DisplayName { get; }

	/// <summary>執行命令；回傳是否成功。</summary>
	bool Execute();
}
