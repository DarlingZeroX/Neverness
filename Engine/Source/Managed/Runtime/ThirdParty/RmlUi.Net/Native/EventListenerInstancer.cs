using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RmlUiNet.Native
{

    internal static unsafe partial class EventListenerInstancer
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_EventListenerInstancer_New")]
        internal static partial IntPtr Create(OnEventListenerInstancer eventListenerInstancer);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_EventListenerInstancer_Free")]
        internal static partial IntPtr Free(IntPtr eventListenerInstancer);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate IntPtr OnEventListenerInstancer(string value, IntPtr element);
    }
}
