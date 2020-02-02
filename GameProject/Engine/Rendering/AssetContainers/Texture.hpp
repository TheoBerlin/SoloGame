#pragma once

#include <Engine/Rendering/AssetContainers/AssetResources.hpp>
#include <atomic>
#include <d3d11.h>

class Texture
{
public:
    Texture()
        :m_RefCount(0),
        m_pSRV(nullptr)
    {}

    Texture(ID3D11ShaderResourceView* pSRV)
        :m_RefCount(0),
        m_pSRV(pSRV)
    {}

    ~Texture()
    {
        if (m_pSRV != nullptr) {
            m_pSRV->Release();
        }
    }

    /*  Can't transfer ownership, texture references would end up referencing an old texture object
        Could implement these in the future, where the functions would copy the texture data,
        without transferring ownership.*/
    Texture(const Texture& texture) = delete;
    void operator=(const Texture& texture) = delete;

    int getRefCount() const
    {
        return m_RefCount.load();
    }

    ID3D11ShaderResourceView* getSRV() const
    {
        return m_pSRV;
    }

    void setSRV(ID3D11ShaderResourceView* pSRV)
    {
        m_pSRV = pSRV;
    }

protected:
    void increaseRefCount()
    {
        m_RefCount += 1;
    }

    void decreaseRefCount()
    {
        m_RefCount -= 1;

        if (m_RefCount == 0 && m_pSRV != nullptr) {
            m_pSRV->Release();
            m_pSRV = nullptr;
        }
    }

private:
    friend class TextureReference;

    ID3D11ShaderResourceView* m_pSRV;
    std::atomic_int m_RefCount;
};

class TextureReference
{
public:
    TextureReference()
        :m_pTexture(nullptr)
    {}

    TextureReference(Texture* pTexture)
        :m_pTexture(pTexture)
    {
        if (pTexture != nullptr) {
            m_pTexture->increaseRefCount();
        }
    }

    ~TextureReference()
    {
        if (m_pTexture != nullptr) {
            m_pTexture->decreaseRefCount();
        }
    }

    TextureReference(const TextureReference& textureReference)
    {
        m_pTexture = textureReference.m_pTexture;

        if (m_pTexture != nullptr) {
            m_pTexture->increaseRefCount();
        }
    }

    void operator=(const TextureReference& textureReference)
    {
        m_pTexture = textureReference.m_pTexture;
        m_pTexture->increaseRefCount();
    }

    ID3D11ShaderResourceView* getSRV() const
    {
        return m_pTexture->m_pSRV;
    }

private:
    Texture* m_pTexture;
};
