#pragma once

#include <Engine/Rendering/APIAbstractions/ResourceFormat.hpp>

#define NOMINMAX
#include <d3d11.h>

DXGI_FORMAT convertFormat(RESOURCE_FORMAT textureFormat);
