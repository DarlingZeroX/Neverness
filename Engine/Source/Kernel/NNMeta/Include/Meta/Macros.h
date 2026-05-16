#pragma once
#include "MetaCommonDefinitions.h"

//#define __REFLECTION_PARSER__
#if defined(__REFLECTION_PARSER__)

#define HMeta(...) __attribute__((annotate(#__VA_ARGS__)))
#define HMetaEnable(...) HMeta(META_Enable,__VA_ARGS__)
#define HMetaDisable() HMeta(META_Disable,__VA_ARGS__)
//#define HEnumClass(...) enum class HMetaEnable(__VA_ARGS__)
//#define HEnum(...) enum HMetaEnable(__VA_ARGS__)
//#define HClass(...) class HMetaEnable(__VA_ARGS__)
//#define HStruct(...) struct HMetaEnable(__VA_ARGS__)

#define HPROPERTY(...) HMetaEnable(__VA_ARGS__)
#define HFUNCTION(...) HMetaEnable(__VA_ARGS__)

#define __META_EXTERNAL(type, guid)       \
        typedef type __META_EXTERNAL__##guid; \

#define _META_EXTERNAL(type, guid) __META_EXTERNAL(type, guid)

#define MetaExternal(type) _META_EXTERNAL(type, __COUNTER__)

#define META_OBJECT

#else

#define HMeta(...)
#define HMetaEnable(...)
#define HMetaDisable()
//#define HEnumClass(...) enum class
//#define HEnum(...) enum
//#define HClass(...) class
//#define HStruct(...) struct

#define HPROPERTY(...)
#define HFUNCTION(...)

#define MetaExternal(type)

#define MetaInitialize(initializer)                               \
        {                                                             \
            auto &db = META_NAMESPACE::ReflectionDatabase::Instance( ); \
            initializer;                                              \
        }                                                             \

// Used in objects to preserve virtual inheritance functionality
#define META_OBJECT                                  \
        META_NAMESPACE::Type GetType(void) const override  \
        {                                                \
            return TypeOf( decltype( *this ) );          \
        }                                                \
        META_NAMESPACE::Object *Clone(void) const override \
        {                                                \
            typedef                                      \
            std::remove_const<                           \
                std::remove_reference<                   \
                    decltype( *this )                    \
                >::type                                  \
            >::type ClassType;                           \
            return new ClassType( *this );               \
        }                                                \

#endif