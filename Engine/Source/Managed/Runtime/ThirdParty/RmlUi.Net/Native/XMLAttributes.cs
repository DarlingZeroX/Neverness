using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RmlUiNet.Native {
    internal static unsafe partial class XMLAttributes {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Dictionary_GetString", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr GetString(IntPtr attributes, string prop, string defaultValue);
    }
}
