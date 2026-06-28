using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace RmlUiNet.Native
{
    internal static unsafe partial class Debugger
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Debugger_Initialise", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Initialise(IntPtr context);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Debugger_SetContext", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void SetContext(IntPtr context);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Debugger_Shutdown", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Shutdown();

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Debugger_IsVisible", StringMarshalling = StringMarshalling.Utf8)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static partial bool IsVisible();

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Debugger_SetVisible", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void SetVisible([MarshalAs(UnmanagedType.U1)] bool visible);
    }
}
