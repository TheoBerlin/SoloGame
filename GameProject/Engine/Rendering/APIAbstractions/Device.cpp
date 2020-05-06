#include "Device.hpp"

#include <Engine/Rendering/APIAbstractions/Texture.hpp>

Device::Device()
    :m_pBackBuffer(nullptr),
    m_pDepthTexture(nullptr)
{}

Device::~Device()
{
    delete m_pBackBuffer;
    delete m_pDepthTexture;
}
