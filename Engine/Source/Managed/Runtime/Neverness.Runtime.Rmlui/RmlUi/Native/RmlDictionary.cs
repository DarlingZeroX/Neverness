using RmlUiNet;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RmlUiNet.Native
{
    internal static unsafe partial class RmlDictionary
    {
        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Dictionary_Create", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr Create();

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Dictionary_Free", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Free(IntPtr dict);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Dictionary_Size", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial int Size(IntPtr dict);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Dictionary_Insert", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial void Insert(IntPtr dict, string prop, IntPtr variant);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Dictionary_Get", StringMarshalling = StringMarshalling.Utf8)]
        internal static partial IntPtr Get(IntPtr dict, string prop);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Dictionary_GetAllKeys", StringMarshalling = StringMarshalling.Utf8)]
        private static partial IntPtr GetAllKeys(IntPtr map, ref int keyCount);

        [LibraryImport(ImportConfig.Native_DLL, EntryPoint = "rml_Dictionary_FreeKeys", StringMarshalling = StringMarshalling.Utf8)]
        private static partial void FreeKeys(IntPtr keys, int keyCount);

        public static string[] RetrieveKeys(IntPtr dict)
        {
            if (dict == IntPtr.Zero)
                return Array.Empty<string>();

            int keyCount = 0;
            IntPtr keysPtr = GetAllKeys(dict, ref keyCount);

            if (keysPtr == IntPtr.Zero || keyCount == 0)
                return Array.Empty<string>();

            try
            {
                string[] keys = new string[keyCount];
                for (int i = 0; i < keyCount; i++)
                {
                    // Read pointer to each string
                    IntPtr stringPtr = Marshal.ReadIntPtr(keysPtr, IntPtr.Size * i);
                    keys[i] = Marshal.PtrToStringAnsi(stringPtr);
                }

                return keys;
            }
            finally
            {
                // Always free the unmanaged memory
                FreeKeys(keysPtr, keyCount);
            }
        }
    }
}
