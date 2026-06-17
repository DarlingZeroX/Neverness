using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Presenters;

namespace Neverness.Editor.AvaloniaFrontend.Controls;

/// <summary>
/// Hosts a shared panel control and safely detaches it from any previous parent
/// before reattaching it when a dockable moves between docked and floating states.
/// </summary>
public class DockContentHost : Decorator
{
    public static readonly StyledProperty<Control?> HostedContentProperty =
        AvaloniaProperty.Register<DockContentHost, Control?>(nameof(HostedContent));

    static DockContentHost()
    {
        HostedContentProperty.Changed.AddClassHandler<DockContentHost>((host, args) =>
        {
            host.OnHostedContentChanged(args.OldValue as Control, args.NewValue as Control);
        });
    }

    public Control? HostedContent
    {
        get => GetValue(HostedContentProperty);
        set => SetValue(HostedContentProperty, value);
    }

    private void OnHostedContentChanged(Control? oldContent, Control? newContent)
    {
        if (ReferenceEquals(oldContent, Child))
        {
            Child = null;
        }

        if (newContent != null)
        {
            DetachFromPreviousParent(newContent);
        }

        Child = newContent;
    }

    private static void DetachFromPreviousParent(Control content)
    {
        switch (content.Parent)
        {
            case ContentPresenter presenter when presenter.TemplatedParent is ContentControl owner:
                owner.Content = null;
                break;

            case ContentControl owner:
                owner.Content = null;
                break;

            case Decorator decorator:
                decorator.Child = null;
                break;

            case Panel panel:
                panel.Children.Remove(content);
                break;
        }
    }
}
