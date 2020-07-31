#include "GeneralResources.hpp"

size_t getFormatSize(RESOURCE_FORMAT format)
{
    switch (format) {
        case RESOURCE_FORMAT::R32G32B32A32_FLOAT:
            return 16;
        case RESOURCE_FORMAT::R32G32B32_FLOAT:
            return 12;
        case RESOURCE_FORMAT::R32G32_FLOAT:
            return 8;
        case RESOURCE_FORMAT::B8G8R8A8_UNORM:
        case RESOURCE_FORMAT::B8G8R8A8_SRGB:
        case RESOURCE_FORMAT::R8G8B8A8_UNORM:
        case RESOURCE_FORMAT::R8G8B8A8_SRGB:
        case RESOURCE_FORMAT::D32_FLOAT:
            return 4;
        default:
            LOG_WARNINGF("Erroneous resource format: %d", (int)format);
            return 16;
    }
}

FORMAT_PRIMITIVE_TYPE getFormatPrimitiveType(RESOURCE_FORMAT format)
{
    switch (format) {
        case RESOURCE_FORMAT::R32G32B32A32_FLOAT:
        case RESOURCE_FORMAT::R32G32B32_FLOAT:
        case RESOURCE_FORMAT::R32G32_FLOAT:
        case RESOURCE_FORMAT::D32_FLOAT:
            return FORMAT_PRIMITIVE_TYPE::FLOAT;
        case RESOURCE_FORMAT::B8G8R8A8_UNORM:
        case RESOURCE_FORMAT::B8G8R8A8_SRGB:
        case RESOURCE_FORMAT::R8G8B8A8_UNORM:
        case RESOURCE_FORMAT::R8G8B8A8_SRGB:
            return FORMAT_PRIMITIVE_TYPE::UNSIGNED_INTEGER;
        default:
            LOG_WARNINGF("Erroneous resource format: %d", (int)format);
            return FORMAT_PRIMITIVE_TYPE::FLOAT;
    }
}
