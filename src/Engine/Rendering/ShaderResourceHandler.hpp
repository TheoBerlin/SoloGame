#pragma once

class Device;
class IBuffer;
class ISampler;

class ShaderResourceHandler
{
public:
    ShaderResourceHandler();
    ~ShaderResourceHandler() = default;

    bool Init(Device* pDevice);
    void Release();

    ISampler* GetAniSampler() { return m_pAniSampler; }

    IBuffer* GetQuarterScreenQuad();

    static ShaderResourceHandler* GetInstance() { return &s_Instance; }

private:
    static ShaderResourceHandler s_Instance;

private:
    Device* m_pDevice;

    ISampler* m_pAniSampler;

    // Quarter-screen quad
    IBuffer* m_pQuadVertices;
};
