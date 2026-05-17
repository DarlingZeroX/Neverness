using Neverness.Managed.Gameplay;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>Phase 6 對白呈現測試（無 Engine ABI 時應安全 no-op）。</summary>
public sealed class DialoguePresenterTests
{
	[Fact]
	public void PresentLine_WhenEngineNotInstalled_ReturnsFalse()
	{
		Assert.False(DialoguePresenter.PresentLine(0, "Hello"));
	}
}
