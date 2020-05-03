#pragma once

#include <Engine/Rendering/AssetContainers/Texture.hpp>

struct ID3D11ShaderResourceView;

class TextureReference
{
public:
    TextureReference();
    TextureReference(Texture* pTexture);

    ~TextureReference();

    TextureReference(const TextureReference& textureReference);

    void operator=(const TextureReference& textureReference);

    ID3D11ShaderResourceView* getSRV() const { return m_pTexture->m_pSRV; }

private:
    Texture* m_pTexture;
};
