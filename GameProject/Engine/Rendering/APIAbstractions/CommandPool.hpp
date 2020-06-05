#pragma once

#include <Engine/Utils/EnumClass.hpp>

#include <stdint.h>

class ICommandList;

enum class COMMAND_POOL_FLAG : uint32_t {
    TEMPORARY_COMMAND_LISTS     = 1,
    RESETTABLE_COMMAND_LISTS    = TEMPORARY_COMMAND_LISTS << 1
};

DEFINE_BITMASK_OPERATIONS(COMMAND_POOL_FLAG)

enum class COMMAND_LIST_LEVEL {
    PRIMARY,
    SECONDARY
};

class ICommandPool
{
public:
    ICommandPool() = default;
    virtual ~ICommandPool() = 0 {};

    virtual bool allocateCommandLists(ICommandList** ppCommandLists, uint32_t commandListCount, COMMAND_LIST_LEVEL level) = 0;
};
