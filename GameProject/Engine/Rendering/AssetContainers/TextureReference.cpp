#include "TextureReference.hpp"

TextureReference::TextureReference()
    :m_pTexture(nullptr)
{}

TextureReference::TextureReference(Texture* pTexture)
    :m_pTexture(pTexture)
{
    if (pTexture != nullptr) {
        m_pTexture->increaseRefCount();
    }
}

TextureReference::~TextureReference()
{
    if (m_pTexture != nullptr) {
        m_pTexture->decreaseRefCount();
    }
}

TextureReference::TextureReference(const TextureReference& textureReference)
{
    m_pTexture = textureReference.m_pTexture;

    if (m_pTexture != nullptr) {
        m_pTexture->increaseRefCount();
    }
}

void TextureReference::operator=(const TextureReference& textureReference)
{
    m_pTexture = textureReference.m_pTexture;
    m_pTexture->increaseRefCount();
}
