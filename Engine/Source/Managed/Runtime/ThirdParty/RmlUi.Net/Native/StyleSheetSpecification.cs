using System;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native
{
    internal static unsafe partial class StyleSheetSpecification
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_StyleSheetSpecification_RegisterProperty", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr RegisterProperty(string name, string defaultValue, [MarshalAs(UnmanagedType.U1)] bool inherited, [MarshalAs(UnmanagedType.U1)] bool forcesLayout);
    }
}
