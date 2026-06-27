using System;
using System.Runtime.InteropServices;
using System.Text;

namespace RmlUiNet.Native {
    internal static unsafe partial class VariableDefinition {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_VariableDefinition_New")]
        internal static partial IntPtr Create(DataVariableType type, OnGet onGet, OnSet onSet, OnSize onSize, OnChild onChild);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_VariableDefinition_Free")]
        internal static partial void Free(IntPtr instance);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate bool OnGet(IntPtr ptr, ref IntPtr variant);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate bool OnSet(IntPtr ptr, IntPtr variant);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int OnSize(IntPtr ptr);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr OnChild(IntPtr ptr, string name, int index);
    }
}
