using System;
using System.Runtime.InteropServices;

namespace RmlUiNet.Native
{
    internal static unsafe partial class Property
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Property_Get", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr Get(IntPtr property);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Property_GetString", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetString(IntPtr property, string defaultValue);
    }
}
