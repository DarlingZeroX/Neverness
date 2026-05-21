using Neverness.Runtime.Engine;
using Neverness.Runtime.Interop;
using Neverness.Runtime.Scene;

namespace Neverness.Runtime.Foundation.Tests;

/// <summary>
/// 校驗 Native <b>NNEntityAPI</b> 子表與託管鏡像（MANAGED 總覽 <b>§2.7.1</b>）：<b>layout v5</b>、<c>getServiceAbiToken</c> 與 <c>getRuntimeTick</c>、可選宿主安裝路徑。
/// </summary>
/// <remarks>
/// 與託管場景 <see cref="SceneEntity"/> 無資料關聯；場景圖由 <c>NNSceneAPI</c> 管理。
/// <c>getRuntimeTick</c> 在 Native 宿主已 <c>Tick</c> 後遞增；僅 <c>dotnet test</c> 時可為 **0**。
/// </remarks>
public sealed class NativeEngineApiEntityServiceTests
{
	/// <summary>與 Native <c>NN_NATIVE_ENGINE_API_LAYOUT_VERSION</c>（<c>EngineAPIRegistry.h</c>）及託管 <see cref="NNNativeEngineApiConstants.LayoutVersion"/> 一致，當前為 <b>10</b>。</summary>
	[Fact]
	public void NativeEngineApi_LayoutVersion_Is10()
	{
		Assert.Equal(10u, NNNativeEngineApiConstants.LayoutVersion);
	}

	/// <summary>
	/// 當 <see cref="EngineNativeApiBootstrap.IsInstalled"/> 為真時，斷言 <c>Entity.GetServiceAbiToken</c> 魔數與 <c>GetRuntimeTick</c> 可呼叫。
	/// </summary>
	[Fact]
	public unsafe void EntityApi_WhenInstalled_GetServiceAbiToken_ReturnsMagic()
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		ref readonly var api = ref EngineNativeApiBootstrap.EngineApi;
		Assert.True(api.Entity.GetServiceAbiToken != null);
		var token = api.Entity.GetServiceAbiToken();
		Assert.Equal(NNNativeEngineApiConstants.EntityServiceAbiToken, token);

		Assert.True(api.Entity.GetRuntimeTick != null);
		_ = api.Entity.GetRuntimeTick();
	}
}
