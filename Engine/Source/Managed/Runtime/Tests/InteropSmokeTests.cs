using Neverness.Runtime.Interop;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>
/// Interop Stub 演练（原 <c>Entry.BootstrapNativeApi</c> 内 <see cref="EngineNativeApiBootstrap.ExerciseStubInteropPath"/>）。
/// </summary>
public sealed class InteropSmokeTests
{
	[Fact]
	public void ExerciseStubInteropPath_when_installed_does_not_throw()
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		EngineNativeApiBootstrap.ExerciseStubInteropPath();
	}
}
