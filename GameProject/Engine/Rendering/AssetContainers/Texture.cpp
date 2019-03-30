#include "Texture.hpp"

#include <Engine/Rendering/AssetLoaders/TextureLoader.hpp>

Texture::Texture(const std::string& filePath, TX_TYPE type)
{
    this->srv = TextureLoader::loadTexture(filePath);

    if (this->srv) {
        this->type = type;
    } else {
        this->type = TX_TYPE::NO_TEXTURE;
    }
}

Texture::~Texture()
{
}

void Texture::setType(TX_TYPE type)
{
    this->type = type;
}

TX_TYPE Texture::getType() const
{
    return this->type;
}

void Texture::setSRV(ID3D11ShaderResourceView* srv)
{
    this->srv = srv;
}

ID3D11ShaderResourceView* Texture::getSRV()
{
    return this->srv;
}
