#pragma once

#include <Engine/Rendering/AssetContainers/AssetResources.hpp>

#include <atomic>

struct ID3D11ShaderResourceView;

class Texture
{
public:
    Texture();
    Texture(ID3D11ShaderResourceView* pSRV);

    ~Texture();

    /*  Can't transfer ownership, texture references would end up referencing an old texture object
        Could implement these in the future, where the functions would copy the texture data,
        without transferring ownership.*/
    Texture(const Texture& texture) = delete;
    void operator=(const Texture& texture) = delete;

    int getRefCount() const { return m_RefCount.load(); }

    ID3D11ShaderResourceView* getSRV() const { return m_pSRV; }
    void setSRV(ID3D11ShaderResourceView* pSRV) { m_pSRV = pSRV; }

protected:
    friend class TextureReference;

    void increaseRefCount() { m_RefCount += 1; }
    void decreaseRefCount();

private:

    ID3D11ShaderResourceView* m_pSRV;
    std::atomic_int m_RefCount;
};
