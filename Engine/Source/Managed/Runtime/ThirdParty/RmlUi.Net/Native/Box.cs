using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RmlUiNet.Native {
    internal static unsafe partial class Box {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Box_GetPosition")]
        internal static partial IntPtr GetPosition(IntPtr box, int area);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Box_GetSize")]
        internal static partial IntPtr GetSize(IntPtr box);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Box_GetSizeBoxArea")]
        internal static partial IntPtr GetSize(IntPtr box, int area);
    }
}
