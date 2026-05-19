using Neverness.Managed.Interop;
using Neverness.Managed.Gameplay;
using SceneModel = Neverness.Managed.Scene.Scene;
using Neverness.Managed.Scene;

namespace Neverness.Managed.Foundation.Tests;

/// <summary>Phase 6：<see cref="SequenceRunner"/> 与步骤之托管行为（含场景再水合联动）。</summary>
public sealed class SequenceRunnerTests
{
	[Fact]
	public void Run_SetVariableSteps_UpdatesStore()
	{
		var store = new GameplayVariableStore();
		var runner = new SequenceRunner(
			new ISequenceStep[]
			{
				new SetVariableSequenceStep("a", 1L),
				new SetVariableSequenceStep("b", "done"),
			});

		Assert.True(runner.Run(store));
		Assert.Equal(1L, store.Get("a"));
		Assert.Equal("done", store.Get("b"));
	}

	[Fact]
	public void Run_WithPresentStep_SucceedsWithoutEngineAbi()
	{
		var store = new GameplayVariableStore();
		var runner = new SequenceRunner(
			new ISequenceStep[]
			{
				new SetVariableSequenceStep("ok", true),
				new PresentDialogueSequenceStep(0, "no engine"),
			});

		Assert.True(runner.Run(store));
		Assert.True((bool)store.Get("ok")!);
	}

	[Fact]
	public void Run_RehydrateSceneThenSyncFirstDisplayName_WritesVariable()
	{
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			return;
		}

		var entity = SceneEntity.Spawn("SceneEntity", "RehydratedTitle");
		if (entity == null)
		{
			return;
		}

		var scene = new SceneModel("DemoScene");
		scene.AddEntity(entity);
		var sceneJson = scene.ToJson();
		entity.Destroy();

		var store = new GameplayVariableStore();
		var runner = new SequenceRunner(
			new ISequenceStep[]
			{
				new RehydrateSceneSequenceStep(sceneJson),
				new SyncFirstEntityDisplayNameToVariableSequenceStep("titleVar"),
			});

		Assert.True(runner.Run(store));
		Assert.Equal("RehydratedTitle", store.Get("titleVar"));
	}

	[Fact]
	public void Run_BranchOnVariable_ExecutesThenPath()
	{
		var store = new GameplayVariableStore();
		store.Set("route", "A");
		var runner = new SequenceRunner(
			new ISequenceStep[]
			{
				new BranchOnVariableSequenceStep(
					"route",
					"A",
					thenSteps: new ISequenceStep[] { new SetVariableSequenceStep("result", "then") },
					elseSteps: new ISequenceStep[] { new SetVariableSequenceStep("result", "else") }),
			});

		Assert.True(runner.Run(store));
		Assert.Equal("then", store.Get("result"));
	}

	[Fact]
	public void Run_BranchOnVariable_UndefinedVariable_ExecutesElsePath()
	{
		var store = new GameplayVariableStore();
		var runner = new SequenceRunner(
			new ISequenceStep[]
			{
				new BranchOnVariableSequenceStep(
					"route",
					"A",
					thenSteps: new ISequenceStep[] { new SetVariableSequenceStep("result", "then") },
					elseSteps: new ISequenceStep[] { new SetVariableSequenceStep("result", "else") }),
			});

		Assert.True(runner.Run(store));
		Assert.Equal("else", store.Get("result"));
	}

	[Fact]
	public void Advance_WaitForVariable_WaitsThenCompletes()
	{
		var store = new GameplayVariableStore();
		var runner = new SequenceRunner(
			new ISequenceStep[]
			{
				new WaitForVariableSequenceStep("click"),
				new SetVariableSequenceStep("done", true),
			});

		var state = new SequenceMachineState(store);
		var r1 = runner.Advance(state);
		Assert.Equal(SequenceAdvanceKind.Waiting, r1.Kind);
		Assert.Equal(0, r1.StepIndex);
		Assert.Equal(0, state.StepIndex);

		store.Set("click", true);
		var r2 = runner.Advance(state);
		Assert.Equal(SequenceAdvanceKind.Advanced, r2.Kind);
		Assert.Equal(1, state.StepIndex);

		var r3 = runner.Advance(state);
		Assert.Equal(SequenceAdvanceKind.Completed, r3.Kind);
		Assert.True((bool)store.Get("done")!);
	}

	[Fact]
	public void Run_WithWaitStep_ReturnsFalseUntilGateOpen()
	{
		var store = new GameplayVariableStore();
		var runner = new SequenceRunner(
			new ISequenceStep[]
			{
				new WaitForVariableSequenceStep("click"),
				new SetVariableSequenceStep("done", 1L),
			});

		Assert.False(runner.Run(store));
		store.Set("click", true);
		Assert.True(runner.Run(store));
		Assert.Equal(1L, store.Get("done"));
	}
}
