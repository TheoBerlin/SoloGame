#pragma once

#include <d3d11.h>

class Display
{
public:
    ~Display();

    static void init(unsigned int height, float aspectRatio);

    ID3D11Device* getDevice();

private:
    static ID3D11Device* device;
};
