#include "BlendState.hpp"

#include <cstring>

BlendState::BlendState(const float pBlendConstants[4])
{
    std::memcpy(m_pBlendConstants, pBlendConstants, sizeof(float) * 4u);
}
