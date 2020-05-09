#include "ResourceFormatDX11.hpp"

DXGI_FORMAT convertFormat(RESOURCE_FORMAT textureFormat)
{
    switch (textureFormat) {
        case RESOURCE_FORMAT::R32G32B32A32_FLOAT:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case RESOURCE_FORMAT::R32G32B32_FLOAT:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case RESOURCE_FORMAT::R32G32_FLOAT:
            return DXGI_FORMAT_R32G32_FLOAT;
        case RESOURCE_FORMAT::R8G8B8A8_UNORM:
            return DXGI_FORMAT_R8G8B8A8_UNORM;
        case RESOURCE_FORMAT::D32_FLOAT:
            return DXGI_FORMAT_D32_FLOAT;
        default:
            LOG_ERROR("Unknown resource format");
            return DXGI_FORMAT_UNKNOWN;
    }
}