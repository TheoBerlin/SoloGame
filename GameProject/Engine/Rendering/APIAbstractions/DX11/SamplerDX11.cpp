#include "SamplerDX11.hpp"

#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

#include <map>

SamplerDX11* SamplerDX11::create(const SamplerInfo& samplerInfo, ID3D11Device* pDevice)
{
    D3D11_SAMPLER_DESC samplerDesc = {};
    setFilterMode(samplerDesc, samplerInfo);
    samplerDesc.AddressU = convertAddressMode(samplerInfo.AddressModeU);
    samplerDesc.AddressV = convertAddressMode(samplerInfo.AddressModeV);
    samplerDesc.AddressW = convertAddressMode(samplerInfo.AddressModeW);
    samplerDesc.MipLODBias = (FLOAT)samplerInfo.MipLODBias;
    samplerDesc.ComparisonFunc = convertComparisonFunc(samplerInfo.ComparisonFunc);

    if (samplerDesc.AddressU == D3D11_TEXTURE_ADDRESS_BORDER || samplerDesc.AddressV == D3D11_TEXTURE_ADDRESS_BORDER || samplerDesc.AddressW == D3D11_TEXTURE_ADDRESS_BORDER) {
        std::memcpy(samplerDesc.BorderColor, samplerInfo.pBorderColor, sizeof(float) * 4u);
    }

    samplerDesc.MinLOD = (FLOAT)samplerInfo.MinLOD;
    samplerDesc.MaxLOD = (FLOAT)samplerInfo.MaxLOD;

    ID3D11SamplerState* pSamplerState = nullptr;
    HRESULT hr = pDevice->CreateSamplerState(&samplerDesc, &pSamplerState);
    if (FAILED(hr)) {
        LOG_WARNING("Failed to create sampler state: %s", hresultToString(hr).c_str());
        SAFERELEASE(pSamplerState)
        return nullptr;
    }

    return new SamplerDX11(pSamplerState);
}

SamplerDX11::SamplerDX11(ID3D11SamplerState* pSamplerState)
    :m_pSamplerState(pSamplerState)
{}

SamplerDX11::~SamplerDX11()
{
    SAFERELEASE(m_pSamplerState)
}

void SamplerDX11::setFilterMode(D3D11_SAMPLER_DESC& samplerDesc, const SamplerInfo& samplerInfo)
{
    samplerDesc.MaxAnisotropy = 1u;

    if (samplerInfo.AnisotropyEnabled) {
        samplerDesc.Filter          = D3D11_FILTER_ANISOTROPIC;
        samplerDesc.MaxAnisotropy   = (UINT)samplerInfo.MaxAnisotropy;
    } else {
        // Map filter combinations to DX11 enumerations (Min, Mag, Mip -> D3D11_FILTER)
        // The filter combinations (keys) follows the pattern of increasing binary numbers (000, 001, 010, 011 etc) where NEAREST is 0 and LIENAR is 1
        std::map<FilterTriplet, D3D11_FILTER, FilterTriplet> filterMap;

        if (samplerInfo.CompareEnabled) {
            filterMap = {
                {{FILTER::NEAREST, FILTER::NEAREST, FILTER::NEAREST}, D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT},
                {{FILTER::NEAREST, FILTER::NEAREST, FILTER::LINEAR}, D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR},
                {{FILTER::NEAREST, FILTER::LINEAR, FILTER::NEAREST}, D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT},
                {{FILTER::NEAREST, FILTER::LINEAR, FILTER::LINEAR}, D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR},
                {{FILTER::LINEAR, FILTER::NEAREST, FILTER::NEAREST}, D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT},
                {{FILTER::LINEAR, FILTER::NEAREST, FILTER::LINEAR}, D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR},
                {{FILTER::LINEAR, FILTER::LINEAR, FILTER::NEAREST}, D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT},
                {{FILTER::LINEAR, FILTER::LINEAR, FILTER::LINEAR}, D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR}
            };
        } else {
            filterMap = {
                {{FILTER::NEAREST, FILTER::NEAREST, FILTER::NEAREST}, D3D11_FILTER_MIN_MAG_MIP_POINT},
                {{FILTER::NEAREST, FILTER::NEAREST, FILTER::LINEAR}, D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR},
                {{FILTER::NEAREST, FILTER::LINEAR, FILTER::NEAREST}, D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT},
                {{FILTER::NEAREST, FILTER::LINEAR, FILTER::LINEAR}, D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR},
                {{FILTER::LINEAR, FILTER::NEAREST, FILTER::NEAREST}, D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT},
                {{FILTER::LINEAR, FILTER::NEAREST, FILTER::LINEAR}, D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR},
                {{FILTER::LINEAR, FILTER::LINEAR, FILTER::NEAREST}, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT},
                {{FILTER::LINEAR, FILTER::LINEAR, FILTER::LINEAR}, D3D11_FILTER_MIN_MAG_MIP_LINEAR}
            };
        }

        auto filterMapItr = filterMap.find({samplerInfo.FilterMin, samplerInfo.FilterMag, samplerInfo.FilterMip});
        if (filterMapItr == filterMap.end()) {
            LOG_WARNING("Erroneous combination of min, mag and mip filters");
            samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        } else {
            samplerDesc.Filter = filterMapItr->second;
        }
    }
}

D3D11_TEXTURE_ADDRESS_MODE SamplerDX11::convertAddressMode(ADDRESS_MODE addressMode)
{
    switch(addressMode) {
        case ADDRESS_MODE::REPEAT:
            return D3D11_TEXTURE_ADDRESS_WRAP;
        case ADDRESS_MODE::MIRROR_REPEAT:
            return D3D11_TEXTURE_ADDRESS_MIRROR;
        case ADDRESS_MODE::MIRROR_CLAMP_TO_EDGE:
            return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
        case ADDRESS_MODE::CLAMP_TO_EDGE:
            return D3D11_TEXTURE_ADDRESS_CLAMP;
        case ADDRESS_MODE::CLAMP_TO_BORDER:
            return D3D11_TEXTURE_ADDRESS_BORDER;
        default:
            LOG_WARNING("Erroneous address mode: %d", (int)addressMode);
            return D3D11_TEXTURE_ADDRESS_WRAP;
    }
}

D3D11_COMPARISON_FUNC SamplerDX11::convertComparisonFunc(COMPARISON_FUNC comparisonFunc)
{
    switch(comparisonFunc) {
        case COMPARISON_FUNC::NEVER:
            return D3D11_COMPARISON_NEVER;
        case COMPARISON_FUNC::LESS:
            return D3D11_COMPARISON_LESS;
        case COMPARISON_FUNC::LESS_OR_EQUAL:
            return D3D11_COMPARISON_LESS_EQUAL;
        case COMPARISON_FUNC::EQUAL:
            return D3D11_COMPARISON_EQUAL;
        case COMPARISON_FUNC::EQUAL_OR_GREATER:
            return D3D11_COMPARISON_GREATER_EQUAL;
        case COMPARISON_FUNC::GREATER:
            return D3D11_COMPARISON_GREATER;
        case COMPARISON_FUNC::ALWAYS:
            return D3D11_COMPARISON_ALWAYS;
        default:
            LOG_WARNING("Erroneous comparison function: %d", (int)comparisonFunc);
            return D3D11_COMPARISON_ALWAYS;
    }
}
