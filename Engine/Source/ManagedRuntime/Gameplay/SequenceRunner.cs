using Neverness.Managed.Engine;

namespace Neverness.Managed.Gameplay;

/// <summary>
/// 序列执行上下文：在同一次 <see cref="SequenceRunner.Run"/> 或 <see cref="SequenceRunner.Advance"/> 推进过程中，共享一套 <see cref="GameplayVariableStore"/>，并可由步骤写入 <see cref="T:Neverness.Managed.Scene.Scene"/>（例如场景 JSON 再水合后供后续步骤读取实体）。
/// </summary>
/// <remarks>
/// <para>Phase 6 slice 3：在纯托管侧把「变量表」与「经 <see cref="T:Neverness.Managed.Scene.SceneRehydrator"/> 恢复的场域实体」放在同一对象中，便于剧本序列编排（不调用 Native <c>VGSceneAPI</c>）。</para>
/// <para>Phase 6 slice 5：<see cref="SequenceMachineState"/> 在多次 <see cref="SequenceRunner.Advance"/> 之间复用同一 <see cref="SequenceContext"/>，因此 <see cref="ActiveScene"/> 与变量在「等待门闩」跨帧期间会一直保持。</para>
/// </remarks>
public sealed class SequenceContext
{
	/// <summary>使用已有变量表构造上下文（通常即当前 Galgame 会话或读档后的变量表）。</summary>
	/// <param name="variables">非空的变量表引用；与 <see cref="SequenceMachineState.Variables"/> 为同一实例。</param>
	public SequenceContext(GameplayVariableStore variables)
	{
		Variables = variables ?? throw new ArgumentNullException(nameof(variables));
	}

	/// <summary>当前剧本变量表；各 <see cref="ISequenceStep"/> 与宿主 UI 可读写，用于分支条件与 <see cref="WaitForVariableSequenceStep"/> 门闩。</summary>
	public GameplayVariableStore Variables { get; }

	/// <summary>
	/// 当前序列关联的托管场景实例；由 <see cref="RehydrateSceneSequenceStep"/> 等步骤赋值。
	/// </summary>
	/// <value>尚未加载或未使用场景时为 <c>null</c>。</value>
	public Neverness.Managed.Scene.Scene? ActiveScene { get; set; }
}

/// <summary>
/// 单步序列接口：由 <see cref="SequenceRunner"/> 按顺序（或按 <see cref="SequenceRunner.Advance"/> 下标）调用 <see cref="Execute"/>。
/// </summary>
/// <remarks>
/// 实现类型见本文件与 <c>SequenceFlow.cs</c>（分支、等待门闩等）。除 <see cref="PresentDialogueSequenceStep"/> 在 ABI 未安装时的约定外，返回 <c>false</c> 通常表示本步失败，调用方应中止或标记 <see cref="SequenceAdvanceKind.Failed"/>。
/// </remarks>
public interface ISequenceStep
{
	/// <summary>执行本步逻辑。</summary>
	/// <param name="context">共享的变量表与可选的 <see cref="SequenceContext.ActiveScene"/>。</param>
	/// <returns>
	/// <c>true</c> 表示本步成功；<c>false</c> 时 <see cref="SequenceRunner.Run"/> 会中止并返回 <c>false</c>。
	/// 对 <see cref="WaitForVariableSequenceStep"/>：门闩未开时返回 <c>false</c>；在 <see cref="SequenceRunner.Advance"/> 路径下同一条件会映射为 <see cref="SequenceAdvanceKind.Waiting"/>（不递增下标）。
	/// </returns>
	/// <remarks>
	/// <see cref="PresentDialogueSequenceStep"/>：若引擎 ABI 未安装则视为跳过（仍返回 <c>true</c>），与 <see cref="DialoguePresenter"/> 的 no-op 语义一致；若已安装则须 <see cref="DialoguePresenter.PresentLine"/> 返回 <c>true</c> 才视为成功。
	/// </remarks>
	bool Execute(SequenceContext context);
}

/// <summary>
/// Phase 6 轻量序列执行器：内存中的步骤列表，不依赖 Native Sequence ABI。
/// </summary>
/// <remarks>
/// Phase 6 slice 5：若步骤列表中含 <see cref="WaitForVariableSequenceStep"/>，线性 <see cref="Run"/> 在门闩未满足时会直接失败；
/// 生产路径请使用 <see cref="Advance"/> + <see cref="SequenceMachineState"/>，在写入变量后反复推进直至 <see cref="SequenceAdvanceKind.Completed"/>。
/// </remarks>
public sealed class SequenceRunner
{
	private readonly ISequenceStep[] _steps;

	/// <summary>从可枚举步骤构造执行器（内部会复制为数组快照，避免运行中被外部修改）。</summary>
	public SequenceRunner(IEnumerable<ISequenceStep> steps)
	{
		ArgumentNullException.ThrowIfNull(steps);
		_steps = steps.ToArray();
	}

	/// <summary>
	/// 取得本执行器包含的步骤数量（构造时快照长度）。
	/// </summary>
	public int StepCount => _steps.Length;

	/// <summary>
	/// 依序执行所有步骤；任一步返回 <c>false</c> 则立即中止并返回 <c>false</c>；全部成功则返回 <c>true</c>。
	/// </summary>
	/// <remarks>
	/// <see cref="WaitForVariableSequenceStep"/>：条件未满足时返回 <c>false</c>（与状态机下 <see cref="SequenceAdvanceKind.Waiting"/> 不同）。
	/// </remarks>
	public bool Run(GameplayVariableStore store)
	{
		ArgumentNullException.ThrowIfNull(store);
		var context = new SequenceContext(store);
		foreach (var step in _steps)
		{
			if (step is WaitForVariableSequenceStep wait)
			{
				if (!wait.Execute(context))
				{
					return false;
				}

				continue;
			}

			if (!step.Execute(context))
			{
				return false;
			}
		}

		return true;
	}

	/// <summary>
	/// 从当前状态前进一步：执行当前下标处的步骤，或在对 <see cref="WaitForVariableSequenceStep"/> 轮询时返回 <see cref="SequenceAdvanceKind.Waiting"/>。
	/// </summary>
	/// <param name="state">由 <see cref="SequenceMachineState"/> 持有的上下文与下标；成功推进时会递增下标。</param>
	/// <returns>本次调用的推进结果（单线程语义；不做任何 Native I/O）。</returns>
	public SequenceAdvanceResult Advance(SequenceMachineState state)
	{
		ArgumentNullException.ThrowIfNull(state);
		if (state.StepIndex >= _steps.Length)
		{
			return new SequenceAdvanceResult(SequenceAdvanceKind.Completed, _steps.Length);
		}

		var step = _steps[state.StepIndex];
		if (step is WaitForVariableSequenceStep wait)
		{
			if (!wait.Execute(state.Context))
			{
				return new SequenceAdvanceResult(SequenceAdvanceKind.Waiting, state.StepIndex);
			}

			state.StepIndex++;
			return state.StepIndex >= _steps.Length
				? new SequenceAdvanceResult(SequenceAdvanceKind.Completed, state.StepIndex)
				: new SequenceAdvanceResult(SequenceAdvanceKind.Advanced, state.StepIndex);
		}

		if (!step.Execute(state.Context))
		{
			return new SequenceAdvanceResult(SequenceAdvanceKind.Failed, state.StepIndex);
		}

		state.StepIndex++;
		return state.StepIndex >= _steps.Length
			? new SequenceAdvanceResult(SequenceAdvanceKind.Completed, state.StepIndex)
			: new SequenceAdvanceResult(SequenceAdvanceKind.Advanced, state.StepIndex);
	}
}

/// <summary>
/// 「设置变量」步骤：向 <see cref="SequenceContext.Variables"/> 写入一个键值，供后续分支、对白或等待门闩读取。
/// </summary>
public sealed class SetVariableSequenceStep : ISequenceStep
{
	private readonly string _name;
	private readonly object? _value;

	/// <summary>创建设置变量步骤。</summary>
	/// <param name="name">变量名，不可为 null 或空白。</param>
	/// <param name="value">变量值；类型建议与 JSON 往返一致（<c>string</c>、<c>bool</c>、<c>long</c>、<c>double</c> 或 <c>null</c>）。</param>
	public SetVariableSequenceStep(string name, object? value)
	{
		_name = name ?? throw new ArgumentNullException(nameof(name));
		_value = value;
	}

	/// <inheritdoc />
	public bool Execute(SequenceContext context)
	{
		context.Variables.Set(_name, _value);
		return true;
	}
}

/// <summary>
/// 「呈现对白」步骤：通过 <see cref="DialoguePresenter"/> 将一行 UTF-16 文本提交到引擎 UI（若已安装相关 Engine 服务函数表）。
/// </summary>
/// <remarks>
/// 未安装 <c>VGNativeEngineAPI</c> 或未接线 UI 时本步仍返回成功，便于纯托管测试与离线工具链；与 <see cref="DialoguePresenter.PresentLine"/> 的 no-op 行为一致。
/// </remarks>
public sealed class PresentDialogueSequenceStep : ISequenceStep
{
	private readonly ulong _elementHandle;
	private readonly string _text;

	/// <summary>创建对白步骤。</summary>
	/// <param name="elementHandle">目标 UI 元素句柄；为 <c>0</c> 时由引擎约定默认元素（若引擎支持）。</param>
	/// <param name="text">对白正文，不可为 null。</param>
	public PresentDialogueSequenceStep(ulong elementHandle, string text)
	{
		_text = text ?? throw new ArgumentNullException(nameof(text));
		_elementHandle = elementHandle;
	}

	/// <inheritdoc />
	public bool Execute(SequenceContext context)
	{
		_ = context;
		if (!EngineNativeApiBootstrap.IsInstalled)
		{
			// 无引擎服务表时不视为失败，便于纯托管单测与离线工具链。
			return true;
		}

		return DialoguePresenter.PresentLine(_elementHandle, _text);
	}
}

/// <summary>
/// 「再水合场景」步骤：将完整场景 JSON 解析并创建 <see cref="T:Neverness.Managed.Scene.Scene"/> 及其实体（经 <see cref="T:Neverness.Managed.Scene.SceneRehydrator"/>），并把结果赋给 <see cref="SequenceContext.ActiveScene"/>。
/// </summary>
/// <remarks>
/// <para>JSON 须由 <see cref="M:Neverness.Managed.Scene.Scene.ToJson"/> 等正规路径产生；反序列化失败或无法创建实体时返回 <c>false</c>。</para>
/// <para>再水合会通过 <see cref="T:Neverness.Managed.Object.LifetimeSystem"/> 注册新的 <see cref="T:Neverness.Managed.Scene.SceneEntity"/>；测试或产品级场景切换时须注意与旧实体 ID 的隔离（测试可调用 <see cref="M:Neverness.Managed.Object.ObjectRegistry.ClearForTesting"/>）。</para>
/// </remarks>
public sealed class RehydrateSceneSequenceStep : ISequenceStep
{
	private readonly string _sceneJson;

	/// <summary>创建再水合场景步骤。</summary>
	/// <param name="sceneJson">完整场景 JSON（含实体条目）。</param>
	public RehydrateSceneSequenceStep(string sceneJson)
	{
		_sceneJson = sceneJson ?? throw new ArgumentNullException(nameof(sceneJson));
	}

	/// <inheritdoc />
	public bool Execute(SequenceContext context)
	{
		ArgumentNullException.ThrowIfNull(context);
		var scene = Neverness.Managed.Scene.SceneRehydrator.RestoreFromJsonWithEntities(_sceneJson);
		if (scene is null)
		{
			return false;
		}

		context.ActiveScene = scene;
		return true;
	}
}

/// <summary>
/// 「同步首实体显示名」步骤：将 <see cref="SequenceContext.ActiveScene"/> 中第一个实体的 <see cref="P:Neverness.Managed.Scene.SceneEntity.DisplayName"/> 写入变量表指定键。
/// </summary>
/// <remarks>
/// 典型用途：再水合后把场景标题或角色名同步到 Galgame 变量表，供 UI 或其它步骤使用。若当前无活动场景或实体列表为空则返回 <c>false</c>。
/// </remarks>
public sealed class SyncFirstEntityDisplayNameToVariableSequenceStep : ISequenceStep
{
	private readonly string _variableName;

	/// <summary>创建同步步骤。</summary>
	/// <param name="variableName">写入 <see cref="GameplayVariableStore"/> 的键名，不可为 null 或空白。</param>
	public SyncFirstEntityDisplayNameToVariableSequenceStep(string variableName)
	{
		_variableName = variableName ?? throw new ArgumentNullException(nameof(variableName));
	}

	/// <inheritdoc />
	public bool Execute(SequenceContext context)
	{
		ArgumentNullException.ThrowIfNull(context);
		var scene = context.ActiveScene;
		if (scene is null || scene.Entities.Count == 0)
		{
			return false;
		}

		context.Variables.Set(_variableName, scene.Entities[0].DisplayName);
		return true;
	}
}
