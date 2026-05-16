namespace VisionGal.Managed.Gameplay;

/// <summary>
/// Phase 6 slice 5：描述 <see cref="SequenceRunner.Advance"/> 单次调用后的推进类别。
/// </summary>
/// <remarks>
/// 与 Native Engine 输入或 Sequence ABI 无关；假定宿主在单线程上循环调用 <see cref="SequenceRunner.Advance"/>，在收到 UI 事件后写入 <see cref="GameplayVariableStore"/> 再继续。
/// </remarks>
public enum SequenceAdvanceKind
{
	/// <summary>已成功执行当前下标对应的一步，且 <see cref="SequenceMachineState.StepIndex"/> 已递增；序列尚未走完。</summary>
	Advanced,

	/// <summary>
	/// 当前步为 <see cref="WaitForVariableSequenceStep"/>，且变量条件尚未满足；<see cref="SequenceMachineState.StepIndex"/> **不**递增。
	/// 宿主应在写入门闩变量后再次调用 <see cref="SequenceRunner.Advance"/>。
	/// </summary>
	Waiting,

	/// <summary>当前步执行失败：普通步骤 <see cref="ISequenceStep.Execute"/> 返回 <c>false</c>，或等待步以外的逻辑失败。</summary>
	Failed,

	/// <summary>所有步骤已执行完毕，序列成功结束；此后再次 <see cref="SequenceRunner.Advance"/> 将仍返回本类别与步骤总数。</summary>
	Completed,
}

/// <summary>
/// <see cref="SequenceRunner.Advance"/> 的返回值：封装 <see cref="SequenceAdvanceKind"/> 与便于诊断的 <see cref="StepIndex"/>。
/// </summary>
public readonly struct SequenceAdvanceResult
{
	/// <summary>构造一次推进结果。</summary>
	/// <param name="kind">本次推进类别。</param>
	/// <param name="stepIndex">
	/// 与 <paramref name="kind"/> 关联的下标：
	/// <see cref="SequenceAdvanceKind.Waiting"/> / <see cref="SequenceAdvanceKind.Failed"/> 时为阻塞或失败位置；
	/// <see cref="SequenceAdvanceKind.Completed"/> 时为步骤总数（已完成）；
	/// <see cref="SequenceAdvanceKind.Advanced"/> 时为递增后的当前下标。
	/// </param>
	public SequenceAdvanceResult(SequenceAdvanceKind kind, int stepIndex)
	{
		Kind = kind;
		StepIndex = stepIndex;
	}

	/// <summary>本次调用的推进类别。</summary>
	public SequenceAdvanceKind Kind { get; }

	/// <summary>与 <see cref="Kind"/> 配套的步骤下标，含义见构造函数参数说明。</summary>
	public int StepIndex { get; }
}

/// <summary>
/// 可恢复序列执行状态：在多次 <see cref="SequenceRunner.Advance"/> 之间保持同一 <see cref="SequenceContext"/>（含 <see cref="SequenceContext.ActiveScene"/> 与变量表）。
/// </summary>
/// <remarks>
/// <para>典型用法：Galgame 宿主在 UI 线程收到「继续」点击后，向 <see cref="Variables"/> 写入门闩键，再调用 <see cref="SequenceRunner.Advance"/>，
/// 直至 <see cref="SequenceAdvanceKind.Completed"/> 或 <see cref="SequenceAdvanceKind.Failed"/>。</para>
/// <para>不要在线程之间共享同一 <see cref="SequenceMachineState"/> 实例而不加同步；本设计面向单线程驱动。</para>
/// </remarks>
public sealed class SequenceMachineState
{
	/// <summary>使用已有变量表创建状态；<see cref="Context"/> 内嵌同一 <see cref="GameplayVariableStore"/>。</summary>
	/// <param name="variables">非空变量表。</param>
	public SequenceMachineState(GameplayVariableStore variables)
	{
		ArgumentNullException.ThrowIfNull(variables);
		Context = new SequenceContext(variables);
	}

	/// <summary>当前待执行步骤在 <see cref="SequenceRunner"/> 内部数组中的下标（从 0 开始）。</summary>
	public int StepIndex { get; set; }

	/// <summary>与本次序列运行绑定的 <see cref="SequenceContext"/>。</summary>
	public SequenceContext Context { get; }

	/// <summary>等价于 <see cref="SequenceContext.Variables"/>，便于宿主在 <see cref="SequenceAdvanceKind.Waiting"/> 时直接写入变量。</summary>
	public GameplayVariableStore Variables => Context.Variables;
}

/// <summary>
/// 变量表标量值的宽松相等比较，与 <see cref="GameplayVariableStore"/> 的 JSON 往返类型集一致。
/// </summary>
/// <remarks>
/// 支持 <c>null</c>、<c>bool</c>、<c>string</c> 的引用相等与值相等；数值型 <c>int</c> / <c>long</c> / <c>double</c> 在双方均可解析为浮点时用误差阈值比较，避免 JSON 往返后的类型差异导致分支误判。
/// </remarks>
internal static class GameplayVariableValueComparer
{
	/// <summary>若两值在宽松规则下视为相等则返回 <c>true</c>。</summary>
	internal static bool LooseEquals(object? a, object? b)
	{
		if (a is null && b is null)
		{
			return true;
		}

		if (a is null || b is null)
		{
			return false;
		}

		if (a.Equals(b))
		{
			return true;
		}

		// 数值：int/long/double 互通比较
		if (TryToDouble(a, out var da) && TryToDouble(b, out var db))
		{
			return Math.Abs(da - db) < 1e-9;
		}

		return false;
	}

	private static bool TryToDouble(object? v, out double d)
	{
		switch (v)
		{
			case double x:
				d = x;
				return true;
			case long x:
				d = x;
				return true;
			case int x:
				d = x;
				return true;
			default:
				d = 0;
				return false;
		}
	}
}

/// <summary>
/// 等待变量表满足条件后再继续：用「轮询变量表」模拟 Galgame 的「等待输入」，不依赖 Native Input 或新的 Sequence ABI。
/// </summary>
/// <remarks>
/// <para><b>与 <see cref="SequenceRunner.Run"/> 的关系</b>：条件未满足时 <see cref="Execute"/> 返回 <c>false</c>，整段线性运行会失败；含本步骤的剧本应使用 <see cref="SequenceRunner.Advance"/>，在 <see cref="SequenceAdvanceKind.Waiting"/> 时不递增下标。</para>
/// <para><b>单参构造函数（真值门闩）</b>：变量键必须已存在，且值为「真」——<c>bool</c> 为 <c>true</c>；整数/浮点非零；非空白字符串；其他已定义类型默认视为真。</para>
/// <para><b>双参构造函数（期望值门闩）</b>：键必须已存在，且与 <paramref name="expected"/> 经 <see cref="GameplayVariableValueComparer.LooseEquals"/> 比较为相等；未定义键一律不满足。</para>
/// </remarks>
public sealed class WaitForVariableSequenceStep : ISequenceStep
{
	private readonly string _name;
	private readonly object? _expected;
	private readonly bool _useTruthiness;

	/// <summary>创建「真值」门闩：变量已定义且按上述真值规则为真时通过。</summary>
	public WaitForVariableSequenceStep(string variableName)
	{
		_name = variableName ?? throw new ArgumentNullException(nameof(variableName));
		ArgumentException.ThrowIfNullOrWhiteSpace(_name);
		_useTruthiness = true;
		_expected = null;
	}

	/// <summary>创建「等于期望值」门闩：变量已定义且与 <paramref name="expected"/> 宽松相等时通过。</summary>
	public WaitForVariableSequenceStep(string variableName, object? expected)
	{
		_name = variableName ?? throw new ArgumentNullException(nameof(variableName));
		ArgumentException.ThrowIfNullOrWhiteSpace(_name);
		_useTruthiness = false;
		_expected = expected;
	}

	/// <inheritdoc />
	public bool Execute(SequenceContext context)
	{
		ArgumentNullException.ThrowIfNull(context);
		if (!context.Variables.TryGet(_name, out var value))
		{
			return false;
		}

		if (_useTruthiness)
		{
			return IsTruthy(value);
		}

		return GameplayVariableValueComparer.LooseEquals(value, _expected);
	}

	private static bool IsTruthy(object? value)
	{
		return value switch
		{
			null => false,
			false => false,
			true => true,
			long l => l != 0,
			int i => i != 0,
			double d => Math.Abs(d) > 1e-9,
			string s => !string.IsNullOrWhiteSpace(s),
			_ => true,
		};
	}
}

/// <summary>
/// 按变量当前值选择执行「则」或「否则」子序列；子步骤在同一 <see cref="SequenceContext"/> 上运行，可读写 <see cref="SequenceContext.ActiveScene"/>。
/// </summary>
/// <remarks>
/// <para>若变量键不存在，走 <paramref name="elseSteps"/>。若存在，则用 <see cref="GameplayVariableValueComparer.LooseEquals"/> 与 <paramref name="matchValue"/> 比较，相等则执行 <paramref name="thenSteps"/>。</para>
/// <para>子序列中若包含 <see cref="WaitForVariableSequenceStep"/>，在单次 <see cref="SequenceRunner.Advance"/> 内仍会一次性执行完该分支内所有步骤（跨帧等待需在分支外编排）。</para>
/// </remarks>
public sealed class BranchOnVariableSequenceStep : ISequenceStep
{
	private readonly string _name;
	private readonly object? _matchValue;
	private readonly ISequenceStep[] _thenSteps;
	private readonly ISequenceStep[] _elseSteps;

	/// <summary>创建分支步骤；子步骤在构造时复制为数组快照。</summary>
	/// <param name="variableName">变量键名。</param>
	/// <param name="matchValue">与变量宽松相等时执行 <paramref name="thenSteps"/>。</param>
	/// <param name="thenSteps">匹配时执行的步骤。</param>
	/// <param name="elseSteps">未定义变量或不匹配时执行的步骤。</param>
	public BranchOnVariableSequenceStep(
		string variableName,
		object? matchValue,
		IEnumerable<ISequenceStep> thenSteps,
		IEnumerable<ISequenceStep> elseSteps)
	{
		_name = variableName ?? throw new ArgumentNullException(nameof(variableName));
		ArgumentException.ThrowIfNullOrWhiteSpace(_name);
		_matchValue = matchValue;
		ArgumentNullException.ThrowIfNull(thenSteps);
		ArgumentNullException.ThrowIfNull(elseSteps);
		_thenSteps = thenSteps.ToArray();
		_elseSteps = elseSteps.ToArray();
	}

	/// <inheritdoc />
	public bool Execute(SequenceContext context)
	{
		ArgumentNullException.ThrowIfNull(context);
		var branch = _elseSteps.AsSpan();
		if (context.Variables.TryGet(_name, out var value) && GameplayVariableValueComparer.LooseEquals(value, _matchValue))
		{
			branch = _thenSteps.AsSpan();
		}

		foreach (var step in branch)
		{
			if (!step.Execute(context))
			{
				return false;
			}
		}

		return true;
	}
}
