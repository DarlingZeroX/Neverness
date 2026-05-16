namespace VisionGal.Managed.Entity;

/// <summary>
/// 簡易本地平移元件：以浮點三元組表達相對於父空間或世界空間之位移（具體座標系由遊戲／場景層約定）。
/// </summary>
/// <remarks>首包刻意不包含旋轉、縮放或矩陣，以降低與 Native 變換管線對齊前之負擔。</remarks>
public sealed class TransformComponent : VGComponent
{
	/// <inheritdoc />
	public TransformComponent(EntityHandle entity)
		: base(entity)
	{
	}

	/// <summary>本地空間或慣用空間下之 X 分量。</summary>
	public float X { get; set; }

	/// <summary>本地空間或慣用空間下之 Y 分量。</summary>
	public float Y { get; set; }

	/// <summary>本地空間或慣用空間下之 Z 分量。</summary>
	public float Z { get; set; }
}
