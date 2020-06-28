#pragma once

#include <type_traits>

#define DEFINE_BITMASK_OPERATIONS(EnumClass)                            \
    inline EnumClass operator |(EnumClass lhs, EnumClass rhs)           \
    {                                                                   \
        return static_cast<EnumClass> (                                 \
            static_cast<std::underlying_type<EnumClass>::type>(lhs) |   \
            static_cast<std::underlying_type<EnumClass>::type>(rhs)     \
        );                                                              \
    }                                                                   \
    inline EnumClass operator &(EnumClass lhs, EnumClass rhs)           \
    {                                                                   \
        return static_cast<EnumClass> (                                 \
            static_cast<std::underlying_type<EnumClass>::type>(lhs) &   \
            static_cast<std::underlying_type<EnumClass>::type>(rhs)     \
        );                                                              \
    }                                                                   \
    inline EnumClass operator ^(EnumClass lhs, EnumClass rhs)           \
    {                                                                   \
        return static_cast<EnumClass> (                                 \
            static_cast<std::underlying_type<EnumClass>::type>(lhs) ^   \
            static_cast<std::underlying_type<EnumClass>::type>(rhs)     \
        );                                                              \
    }                                                                   \
    inline EnumClass operator ~(EnumClass rhs)                          \
    {                                                                   \
        return static_cast<EnumClass> (                                 \
            ~static_cast<std::underlying_type<EnumClass>::type>(rhs)    \
        );                                                              \
    }                                                                   \
    inline EnumClass& operator |=(EnumClass &lhs, EnumClass rhs)        \
    {                                                                   \
        lhs = static_cast<EnumClass> (                                  \
            static_cast<std::underlying_type<EnumClass>::type>(lhs) |   \
            static_cast<std::underlying_type<EnumClass>::type>(rhs)     \
        );                                                              \
                                                                        \
        return lhs;                                                     \
    }                                                                   \
    inline EnumClass& operator &=(EnumClass &lhs, EnumClass rhs)        \
    {                                                                   \
        lhs = static_cast<EnumClass> (                                  \
            static_cast<std::underlying_type<EnumClass>::type>(lhs) &   \
            static_cast<std::underlying_type<EnumClass>::type>(rhs)     \
        );                                                              \
                                                                        \
        return lhs;                                                     \
    }                                                                   \
    inline EnumClass& operator ^=(EnumClass &lhs, EnumClass rhs)        \
    {                                                                   \
        lhs = static_cast<EnumClass> (                                  \
            static_cast<std::underlying_type<EnumClass>::type>(lhs) ^   \
            static_cast<std::underlying_type<EnumClass>::type>(rhs)     \
        );                                                              \
                                                                        \
        return lhs;                                                     \
    }

// Check if the bitmask contains the flag
#define HAS_FLAG(bitmask, flag) (flag == ((bitmask) & (flag)))
