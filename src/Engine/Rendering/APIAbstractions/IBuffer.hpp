#pragma once

#include <Engine/Rendering/APIAbstractions/GeneralResources.hpp>
#include <Engine/Utils/EnumClass.hpp>

#include <stdint.h>

class IBuffer;
class ICommandList;

struct StagingResources {
    ICommandList* pCommandList;
    IBuffer* pStagingBuffer;
};

enum class BUFFER_DATA_ACCESS : uint32_t {
    NONE    = 0,
    READ    = 1,
    WRITE   = READ << 1
};

DEFINE_BITMASK_OPERATIONS(BUFFER_DATA_ACCESS)

enum class BUFFER_USAGE : uint32_t {
    VERTEX_BUFFER   = 1,
    INDEX_BUFFER    = VERTEX_BUFFER << 1,
    UNIFORM_BUFFER  = INDEX_BUFFER  << 1,
    STAGING_BUFFER  = UNIFORM_BUFFER << 1
};

struct BufferInfo {
    size_t ByteSize;
    const void* pData;
    BUFFER_DATA_ACCESS CPUAccess;
    BUFFER_DATA_ACCESS GPUAccess;
    BUFFER_USAGE Usage;
    SHARING_MODE SharingMode;
    std::vector<uint32_t> QueueFamilyIndices;
};

class IBuffer
{
public:
    virtual ~IBuffer() = 0 {};
};
