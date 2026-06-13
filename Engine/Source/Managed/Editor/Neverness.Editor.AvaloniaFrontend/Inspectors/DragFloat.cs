using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.VisualTree;

namespace Neverness.Editor.AvaloniaFrontend.Inspectors;

/// <summary>
/// DragFloat 控件——类似 ImGui 的 DragFloat 行为。
///
/// - 鼠标左键拖动：左右拖动调整数值
/// - 双击：进入编辑模式，可直接输入数值
/// - 无上下按钮，无滑块
/// </summary>
public class DragFloat : Border
{
    // ── 依赖属性 ──
    public static readonly StyledProperty<float> ValueProperty =
        AvaloniaProperty.Register<DragFloat, float>(nameof(Value), default(float));

    public static readonly StyledProperty<float> SpeedProperty =
        AvaloniaProperty.Register<DragFloat, float>(nameof(Speed), 0.1f);

    public static readonly StyledProperty<float> MinProperty =
        AvaloniaProperty.Register<DragFloat, float>(nameof(Min), float.MinValue);

    public static readonly StyledProperty<float> MaxProperty =
        AvaloniaProperty.Register<DragFloat, float>(nameof(Max), float.MaxValue);

    public static readonly StyledProperty<string> FormatProperty =
        AvaloniaProperty.Register<DragFloat, string>(nameof(Format), "F2");

    public float Value
    {
        get => GetValue(ValueProperty);
        set => SetValue(ValueProperty, value);
    }

    public float Speed
    {
        get => GetValue(SpeedProperty);
        set => SetValue(SpeedProperty, value);
    }

    public float Min
    {
        get => GetValue(MinProperty);
        set => SetValue(MinProperty, value);
    }

    public float Max
    {
        get => GetValue(MaxProperty);
        set => SetValue(MaxProperty, value);
    }

    public string Format
    {
        get => GetValue(FormatProperty);
        set => SetValue(FormatProperty, value);
    }

    // ── 内部状态 ──
    private TextBlock? _textBlock;
    private TextBox? _editBox;
    private bool _isDragging;
    private bool _isEditing;
    private Point _dragStart;
    private float _dragStartValue;
    private EventHandler<PointerPressedEventArgs>? _globalPointerHandler;

    // 毛玻璃半透明背景
    private static readonly SolidColorBrush BgNormal = new(Color.FromArgb(180, 55, 55, 60));
    private static readonly SolidColorBrush BgHover = new(Color.FromArgb(200, 70, 70, 78));
    private static readonly SolidColorBrush BgDrag = new(Color.FromArgb(220, 0, 120, 212));
    private static readonly SolidColorBrush TextColor = new(Color.Parse("#FFCCCCCC"));

    public DragFloat()
    {
        Background = BgNormal;
        CornerRadius = new CornerRadius(3);
        Padding = new Thickness(6, 2);
        MinWidth = 60;
        Height = 22;
        Cursor = new Cursor(StandardCursorType.SizeWestEast);
        Focusable = true;

        // 文本居中显示
        _textBlock = new TextBlock
        {
            Text = Value.ToString(Format),
            FontSize = 12,
            HorizontalAlignment = Avalonia.Layout.HorizontalAlignment.Stretch,
            VerticalAlignment = Avalonia.Layout.VerticalAlignment.Center,
            TextAlignment = Avalonia.Media.TextAlignment.Center,
            Foreground = TextColor,
            FontFamily = new FontFamily("Consolas"),
        };
        Child = _textBlock;

        // 事件
        PointerEntered += OnPointerEntered;
        PointerExited += OnPointerExited;
        PointerPressed += OnPointerPressed;
        PointerMoved += OnPointerMoved;
        PointerReleased += OnPointerReleased;
        DoubleTapped += OnDoubleTapped;
        LostFocus += OnLostFocus;

        // 属性变更
        PropertyChanged += (_, e) =>
        {
            if (e.Property == ValueProperty && !_isDragging && !_isEditing)
                UpdateText();
        };
    }

    private void UpdateText()
    {
        if (_textBlock != null && !_isEditing)
            _textBlock.Text = Value.ToString(Format);
    }

    private void OnPointerEntered(object? sender, PointerEventArgs e)
    {
        if (!_isDragging && !_isEditing && !IsFocused)
            Background = BgHover;
    }

    private void OnPointerExited(object? sender, PointerEventArgs e)
    {
        if (!_isDragging && !_isEditing && !IsFocused)
            Background = BgNormal;
    }

    private void OnLostFocus(object? sender, RoutedEventArgs e)
    {
        // DragFloat 失焦时恢复普通状态，但不提交编辑（TextBox 的 LostFocus 会处理）
        if (!_isDragging && !_isEditing)
            Background = BgNormal;
    }

    private void OnPointerPressed(object? sender, PointerPressedEventArgs e)
    {
        var props = e.GetCurrentPoint(this).Properties;
        if (!props.IsLeftButtonPressed) return;

        // 如果正在编辑，检查点击目标是否是 TextBox
        if (_isEditing)
        {
            var source = e.Source as Control;
            // 点击的是 TextBox 区域，不处理（让 TextBox 继续编辑）
            if (source == _editBox || _editBox?.IsVisualAncestorOf(source) == true)
                return;
            // 点击的是 DragFloat 其他区域，提交编辑
            CommitEdit();
            return;
        }

        // 获取焦点（这样点击其他地方才能触发 LostFocus）
        Focus();

        _isDragging = true;
        _dragStart = e.GetPosition(this);
        _dragStartValue = Value;
        Background = BgDrag;
        e.Pointer.Capture(this);
    }

    private void OnPointerMoved(object? sender, PointerEventArgs e)
    {
        if (!_isDragging) return;

        var pos = e.GetPosition(this);
        var dx = pos.X - _dragStart.X;

        // 拖动距离 * 速度 = 数值变化
        var newValue = _dragStartValue + (float)dx * Speed;
        newValue = Math.Clamp(newValue, Min, Max);
        Value = newValue;

        _textBlock!.Text = Value.ToString(Format);
    }

    private void OnPointerReleased(object? sender, PointerReleasedEventArgs e)
    {
        if (!_isDragging) return;

        _isDragging = false;
        Background = BgHover;
        e.Pointer.Capture(null);
    }

    private void OnDoubleTapped(object? sender, TappedEventArgs e)
    {
        if (_isEditing) return;

        _isEditing = true;
        Background = BgDrag;

        // 替换为 TextBox
        var editBox = new TextBox
        {
            Text = Value.ToString(Format),
            FontSize = 12,
            FontFamily = new FontFamily("Consolas"),
            Foreground = TextColor,
            Background = Brushes.Transparent,
            BorderThickness = new Thickness(0),
            TextAlignment = Avalonia.Media.TextAlignment.Center,
            MinWidth = 50,
        };
        _editBox = editBox;
        Child = editBox;

        editBox.KeyDown += (_, ke) =>
        {
            if (ke.Key == Key.Enter)
                CommitEdit();
            else if (ke.Key == Key.Escape)
                CancelEdit();
        };

        editBox.LostFocus += (_, _) => CommitEdit();

        editBox.Focus();
        editBox.SelectAll();

        // 注册全局指针监听：点击任何非 TextBox 区域就提交编辑
        _globalPointerHandler = (_, pe) =>
        {
            if (!_isEditing || _editBox == null) return;
            // 点击的是 TextBox 区域，不处理
            var source = pe.Source as Control;
            if (source == _editBox || (source != null && _editBox.IsVisualAncestorOf(source)))
                return;
            CommitEdit();
        };
        // 在 Window 级别添加事件处理
        var topLevel = TopLevel.GetTopLevel(this);
        if (topLevel != null)
            topLevel.AddHandler(PointerPressedEvent, _globalPointerHandler, Avalonia.Interactivity.RoutingStrategies.Bubble);
    }

    private void CommitEdit()
    {
        if (!_isEditing || _editBox == null) return;

        if (float.TryParse(_editBox.Text, out var newValue))
            Value = Math.Clamp(newValue, Min, Max);

        _isEditing = false;
        _editBox = null;
        Child = _textBlock;
        UpdateText();
        Background = BgNormal;
        UnregisterGlobalPointer();
    }

    private void CancelEdit()
    {
        _isEditing = false;
        _editBox = null;
        Child = _textBlock;
        UpdateText();
        Background = BgNormal;
        UnregisterGlobalPointer();
    }

    private void UnregisterGlobalPointer()
    {
        if (_globalPointerHandler != null)
        {
            var topLevel = TopLevel.GetTopLevel(this);
            topLevel?.RemoveHandler(PointerPressedEvent, _globalPointerHandler);
            _globalPointerHandler = null;
        }
    }
}
