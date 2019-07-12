#pragma once

#include <Engine/Rendering/AssetContainers/AssetResources.hpp>
#include <d3d11.h>

enum TX_TYPE {
    // Indicates a texture failed to load or has yet to be loaded
    NO_TEXTURE,
    DIFFUSE = aiTextureType_DIFFUSE,
    NORMAL = aiTextureType_NORMALS
};

class Texture
{
public:
    Texture(const std::string& filePath, TX_TYPE type);
    ~Texture();

    void setType(TX_TYPE type);
    TX_TYPE getType() const;

    void setSRV(ID3D11ShaderResourceView* srv);
    ID3D11ShaderResourceView* getSRV();

private:
    TX_TYPE type;
    ID3D11ShaderResourceView* srv;
};
