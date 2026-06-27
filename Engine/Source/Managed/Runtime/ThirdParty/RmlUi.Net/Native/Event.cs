using System;
using System.Runtime.InteropServices;
using System.Text;

namespace RmlUiNet.Native
{
    internal static unsafe partial class Event
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Event_GetId", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial ushort GetId(IntPtr ev);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Event_GetPhase", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial int GetPhase(IntPtr ev);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Event_GetCurrentElement", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetCurrentElement(IntPtr ev, out IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Event_GetTargetElement", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetTargetElement(IntPtr ev, out IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Event_StopPropagation", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void StopPropagation(IntPtr ev);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Event_StopImmediatePropagation", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void StopImmediatePropagation(IntPtr ev);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Event_IsInterruptible", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool IsInterruptible(IntPtr ev);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Event_IsPropagating", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool IsPropagating(IntPtr ev);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Event_IsImmediatePropagating", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool IsImmediatePropagating(IntPtr ev);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Event_GetParameters", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetParameters(IntPtr element);
    }
}
