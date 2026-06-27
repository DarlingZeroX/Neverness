using System;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native
{
    internal static unsafe partial class ElementFormControlInput
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementFormControlInput_GetValue", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetValue(IntPtr element);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementFormControlInput_SetValue", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void SetValue(IntPtr element, string value);
    }
}
