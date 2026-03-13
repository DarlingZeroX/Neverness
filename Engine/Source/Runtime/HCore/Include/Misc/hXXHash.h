#pragma once

#include "../HCoreTypes.h"
#include <string>
#include <assert.h>

typedef struct XXH64_state_s XXH64_state_t;   /* incomplete type */

namespace Horizon
{
    class vaXXHash64
    {
    private:
        XXH64_state_t* m_state = nullptr;

    public:
        vaXXHash64(uint64 seed = 0);
        ~vaXXHash64();

        vaXXHash64(const vaXXHash64& copy) = delete;
        vaXXHash64& operator = (const vaXXHash64& copy) = delete;

    public:
        // compute xxhash directly from input buffer
        static uint64       Compute(const void* dataPtr, int64 length, uint64 seed = 0);

        // get current value
        //operator             uint64( ) const                  { return Digest(); }
        uint64              Digest() const;

        void                AddBytes(const void* dataPtr, int64 length);

        inline void         AddString(const std::string& str)
        {
            int length = (int)str.length();
            AddValue(length);
            if (length > 0)  AddBytes(str.c_str(), length);
        }

        inline void         AddString(const std::wstring& str)
        {
            int length = (int)str.length() * 2;
            AddValue(length);
            if (length > 0)  AddBytes(str.c_str(), length);
        }

        template< class ValueType >
        inline void         AddValue(ValueType val)
        {
            AddBytes(&val, sizeof(val));
        }

    private:

    };

}
