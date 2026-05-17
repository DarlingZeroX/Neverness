using Neverness.Managed.Engine;
using Neverness.Managed.Object;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>託管物件註冊表與測試清理行為。</summary>
public sealed class ObjectRegistryTests
{
	private sealed class TestVGObject : VGObject
	{
		public TestVGObject(VGObjectId id, VGObjectHandle handle)
			: base(id, handle)
		{
		}

		public override string TypeName => "TestVGObject";
	}

	[Fact]
	public void ClearForTesting_RemovesAllRegistrations()
	{
		ObjectRegistry.ClearForTesting();
		var obj = new TestVGObject(new VGObjectId(99), new VGObjectHandle(99));
		ObjectRegistry.Register(obj);
		Assert.Equal(1, ObjectRegistry.Count);

		ObjectRegistry.ClearForTesting();

		Assert.Equal(0, ObjectRegistry.Count);
		Assert.False(ObjectRegistry.TryGet(new VGObjectId(99), out _));
	}
}
