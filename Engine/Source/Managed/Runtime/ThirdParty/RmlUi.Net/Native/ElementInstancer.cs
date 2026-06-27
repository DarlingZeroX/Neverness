using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RmlUiNet.Native {

    internal static unsafe partial class ElementInstancer {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_ElementInstancer_New", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr Create(string tag, OnInstanceElement onInstanceElement, OnReleaseElement onReleaseElement);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate IntPtr OnInstanceElement(IntPtr parentElement, string tag, IntPtr xmlAttributes);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnReleaseElement(IntPtr element);
    }
}
