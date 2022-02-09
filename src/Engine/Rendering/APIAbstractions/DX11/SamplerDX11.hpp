#pragma once

#include <Engine/Rendering/APIAbstractions/ISampler.hpp>

#include <d3d11.h>
#include <type_traits>

class SamplerDX11 : public ISampler
{
public:
    static SamplerDX11* create(const SamplerInfo& samplerInfo, ID3D11Device* pDevice);

public:
    SamplerDX11(ID3D11SamplerState* pSamplerState);
    ~SamplerDX11();

    ID3D11SamplerState* getSamplerState() { return m_pSamplerState; }

private:
    struct FilterTriplet {
        bool operator ()(const FilterTriplet& left, const FilterTriplet& right) const
        {
            // First compare min, then mag and lastly mip
            if (left.FilterMin == right.FilterMin) {
                if (left.FilterMag == right.FilterMag) {
                    return left.FilterMip <= right.FilterMip;
                } else {
                    return left.FilterMag < right.FilterMag;
                }
            } else {
                return left.FilterMin < right.FilterMin;
            }
        }

        FILTER FilterMin;
        FILTER FilterMag;
        FILTER FilterMip;
    };

private:
    static void setFilterMode(D3D11_SAMPLER_DESC& samplerDesc, const SamplerInfo& samplerInfo);
    static D3D11_TEXTURE_ADDRESS_MODE convertAddressMode(ADDRESS_MODE addressMode);
    static D3D11_COMPARISON_FUNC convertComparisonFunc(COMPARISON_FUNC comparisonFunc);
    static void convertBorderColor(BORDER_COLOR borderColor, FLOAT* pColor);

private:
    ID3D11SamplerState* m_pSamplerState;
};
