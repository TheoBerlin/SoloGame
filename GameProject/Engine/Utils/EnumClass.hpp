#pragma once

#include <type_traits>

#define DECLARE_BITMASK(EnumClass)                          \
    EnumClass operator |(EnumClass lhs, EnumClass rhs);     \
    EnumClass operator &(EnumClass lhs, EnumClass rhs);     \
    EnumClass operator ^(EnumClass lhs, EnumClass rhs);     \
    EnumClass operator ~(EnumClass rhs);                    \
    EnumClass& operator |=(EnumClass &lhs, EnumClass rhs);  \
    EnumClass& operator &=(EnumClass &lhs, EnumClass rhs);  \
    EnumClass& operator ^=(EnumClass &lhs, EnumClass rhs);

#define DEFINE_BITMASK(EnumClass)                                       \
    EnumClass operator |(EnumClass lhs, EnumClass rhs)                  \
    {                                                                   \
        return static_cast<EnumClass> (                                 \
            static_cast<std::underlying_type<EnumClass>::type>(lhs) |   \
            static_cast<std::underlying_type<EnumClass>::type>(rhs)     \
        );                                                              \
    }                                                                   \
    EnumClass operator &(EnumClass lhs, EnumClass rhs)                  \
    {                                                                   \
        return static_cast<EnumClass> (                                 \
            static_cast<std::underlying_type<EnumClass>::type>(lhs) &   \
            static_cast<std::underlying_type<EnumClass>::type>(rhs)     \
        );                                                              \
    }                                                                   \
    EnumClass operator ^(EnumClass lhs, EnumClass rhs)                  \
    {                                                                   \
        return static_cast<EnumClass> (                                 \
            static_cast<std::underlying_type<EnumClass>::type>(lhs) ^   \
            static_cast<std::underlying_type<EnumClass>::type>(rhs)     \
        );                                                              \
    }                                                                   \
    EnumClass operator ~(EnumClass rhs)                                 \
    {                                                                   \
        return static_cast<EnumClass> (                                 \
            ~static_cast<std::underlying_type<EnumClass>::type>(rhs)    \
        );                                                              \
    }                                                                   \
    EnumClass& operator |=(EnumClass &lhs, EnumClass rhs)               \
    {                                                                   \
        lhs = static_cast<EnumClass> (                                  \
            static_cast<std::underlying_type<EnumClass>::type>(lhs) |   \
            static_cast<std::underlying_type<EnumClass>::type>(rhs)     \
        );                                                              \
                                                                        \
        return lhs;                                                     \
    }                                                                   \
    EnumClass& operator &=(EnumClass &lhs, EnumClass rhs)               \
    {                                                                   \
        lhs = static_cast<EnumClass> (                                  \
            static_cast<std::underlying_type<EnumClass>::type>(lhs) &   \
            static_cast<std::underlying_type<EnumClass>::type>(rhs)     \
        );                                                              \
                                                                        \
        return lhs;                                                     \
    }                                                                   \
    EnumClass& operator ^=(EnumClass &lhs, EnumClass rhs)               \
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
