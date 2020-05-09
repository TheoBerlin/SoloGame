#include "DeviceDX11.hpp"

#include <Engine/Rendering/APIAbstractions/DX11/CommandListDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/InputLayoutDX11.hpp>
#include <Engine/Rendering/APIAbstractions/DX11/RasterizerStateDX11.hpp>
#include <Engine/Rendering/Window.hpp>
#include <Engine/Utils/DirectXUtils.hpp>
#include <Engine/Utils/Logger.hpp>

DeviceDX11::DeviceDX11()
    :m_pDevice(nullptr),
    m_pSwapChain(nullptr),
    m_pContext(nullptr),
    m_pDepthStencilState(nullptr)
{}

DeviceDX11::~DeviceDX11()
{
    SAFERELEASE(m_pDevice)
    SAFERELEASE(m_pSwapChain)
    SAFERELEASE(m_pContext)
    SAFERELEASE(m_pDepthStencilState)
}

bool DeviceDX11::init(const SwapChainInfo& swapChainInfo, Window* pWindow)
{
    if (!initDeviceAndSwapChain(swapChainInfo, pWindow)) {
        return false;
    }

    return initBackBuffers(swapChainInfo, pWindow);
}

void DeviceDX11::clearBackBuffer()
{
    TextureDX11* pBackBuffer = reinterpret_cast<TextureDX11*>(m_pBackBuffer);
    TextureDX11* pDepthTexture = reinterpret_cast<TextureDX11*>(m_pDepthTexture);

    m_pContext->ClearRenderTargetView(pBackBuffer->getRTV(), m_pClearColor);
    m_pContext->ClearDepthStencilView(pDepthTexture->getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);
}

void DeviceDX11::presentBackBuffer()
{
    HRESULT hr = m_pSwapChain->Present(0, 0);
    if (FAILED(hr)) {
        LOG_WARNING("Failed to present swap-chain buffer: %s", hresultToString(hr).c_str());
    }
}

ICommandList* DeviceDX11::createCommandList()
{
    CommandListDX11* pCommandList = new CommandListDX11(m_pContext, m_pDevice);
    if (!pCommandList->getContext()) {
        delete pCommandList;
        pCommandList = nullptr;
    }

    return pCommandList;
}

BufferDX11* DeviceDX11::createBuffer(const BufferInfo& bufferInfo)
{
    BufferDX11* pBuffer = new BufferDX11(m_pDevice, bufferInfo);
    if (!pBuffer->getBuffer()) {
        delete pBuffer;
        pBuffer = nullptr;
    }

    return pBuffer;
}

BufferDX11* DeviceDX11::createVertexBuffer(const void* pVertices, size_t vertexSize, size_t vertexCount)
{
    BufferInfo bufferInfo   = {};
    bufferInfo.ByteSize     = vertexSize * vertexCount;
    bufferInfo.pData        = pVertices;
    bufferInfo.Usage        = BUFFER_USAGE::VERTEX_BUFFER;
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::READ;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::NONE;

    return createBuffer(bufferInfo);
}

BufferDX11* DeviceDX11::createIndexBuffer(const unsigned* pIndices, size_t indexCount)
{
    DeviceDX11* pDeviceDX = reinterpret_cast<DeviceDX11*>(m_pDevice);
    ID3D11Device* pDevice = pDeviceDX->getDevice();

    BufferInfo bufferInfo   = {};
    bufferInfo.ByteSize     = sizeof(unsigned) * indexCount;
    bufferInfo.pData        = pIndices;
    bufferInfo.Usage        = BUFFER_USAGE::INDEX_BUFFER;
    bufferInfo.GPUAccess    = BUFFER_DATA_ACCESS::READ;
    bufferInfo.CPUAccess    = BUFFER_DATA_ACCESS::NONE;

    return createBuffer(bufferInfo);
}

TextureDX11* DeviceDX11::createTextureFromFile(const std::string& filePath)
{
    return TextureDX11::createFromFile(filePath, m_pDevice);
}

TextureDX11* DeviceDX11::createTexture(const TextureInfo& textureInfo)
{
    return TextureDX11::create(textureInfo, m_pDevice);
}

SamplerDX11* DeviceDX11::createSampler(const SamplerInfo& samplerInfo)
{
    return SamplerDX11::create(samplerInfo, m_pDevice);
}

IRasterizerState* DeviceDX11::createRasterizerState(const RasterizerStateInfo& rasterizerInfo)
{
    return RasterizerStateDX11::create(rasterizerInfo, m_pDevice);
}

DepthStencilStateDX11* DeviceDX11::createDepthStencilState(const DepthStencilInfo& depthStencilInfo)
{
    return DepthStencilStateDX11::create(depthStencilInfo, m_pDevice);
}

BlendStateDX11* DeviceDX11::createBlendState(const BlendStateInfo& blendStateInfo)
{
    return BlendStateDX11::create(blendStateInfo, m_pDevice);
}

bool DeviceDX11::initDeviceAndSwapChain(const SwapChainInfo& swapChainInfo, Window* pWindow)
{
    UINT deviceFlags = 0;

    // If in debug mode, turn on D3D11 debugging
    #if defined _DEBUG
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    // Determines the order of feature levels to attempt to create
    D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};

    // Stores the feature level that was created
    D3D_FEATURE_LEVEL createdFeatureLevel;

    // Create swap chain description
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferDesc.Width  = (UINT)pWindow->getWidth();
    swapChainDesc.BufferDesc.Height = (UINT)pWindow->getHeight();
    swapChainDesc.BufferDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering   = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling            = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.BufferDesc.RefreshRate.Numerator      = (UINT)swapChainInfo.FrameRateLimit;
    swapChainDesc.BufferDesc.RefreshRate.Denominator    = 1;
    swapChainDesc.SampleDesc.Count              = (UINT)swapChainInfo.Multisamples;
    swapChainDesc.SampleDesc.Quality            = 0;
    swapChainDesc.BufferUsage   = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
    swapChainDesc.BufferCount   = 2;
    swapChainDesc.OutputWindow  = pWindow->getHWND();
    swapChainDesc.Windowed      = swapChainInfo.Windowed;
    swapChainDesc.SwapEffect    = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    // Create device
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        deviceFlags,
        featureLevels,
        sizeof(featureLevels) / sizeof(featureLevels[0]),
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &m_pSwapChain,
        &m_pDevice,
        &createdFeatureLevel,
        &m_pContext
    );

    if (FAILED(hr) || !m_pDevice || !m_pSwapChain || !m_pContext) {
        LOG_ERROR("Failed to create Device and Swap Chain: %s", hresultToString(hr).c_str());
        return false;
    }

    return true;
}

bool DeviceDX11::initBackBuffers(const SwapChainInfo& swapChainInfo, Window* pWindow)
{
    // Create render target view from swap chain's back buffer
    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to retrieve Swap Chain's back buffer: %s", hresultToString(hr).c_str());
        return false;
    }

    ID3D11RenderTargetView* pBackBufferRTV = nullptr;
    hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pBackBufferRTV);
    pBackBuffer->Release();

    if (FAILED(hr)) {
        LOG_ERROR("Failed to create Render Target: %s", hresultToString(hr).c_str());
        return false;
    }

    D3D11_TEXTURE2D_DESC backBufferDesc = {};
    pBackBuffer->GetDesc(&backBufferDesc);
    m_pBackBuffer = new TextureDX11({(uint32_t)backBufferDesc.Width, (uint32_t)backBufferDesc.Height}, nullptr, nullptr, pBackBufferRTV, nullptr);

    /* Depth stencil */
    TextureInfo textureInfo = {};
    textureInfo.Dimensions      = {pWindow->getWidth(), pWindow->getHeight()};
    textureInfo.Format          = RESOURCE_FORMAT::D32_FLOAT;
    textureInfo.InitialLayout   = TEXTURE_LAYOUT::DEPTH_ATTACHMENT;
    textureInfo.LayoutFlags     = textureInfo.InitialLayout;

    m_pDepthTexture = createTexture(textureInfo);
    if (!m_pDepthTexture) {
        LOG_ERROR("Failed to create depth texture");
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable                  = true;
    dsDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc                    = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable                = false;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_LESS;
    dsDesc.BackFace                     = dsDesc.FrontFace;

    hr = m_pDevice->CreateDepthStencilState(&dsDesc, &m_pDepthStencilState);
    if (FAILED(hr)) {
        LOG_ERROR("Failed to create depth stencil state: %s", hresultToString(hr).c_str());
        return false;
    }

    TextureDX11* pDepthTexture = reinterpret_cast<TextureDX11*>(m_pDepthTexture);
    m_pContext->ClearDepthStencilView(pDepthTexture->getDSV(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0);

    return true;
}

ShaderDX11* DeviceDX11::compileShader(SHADER_TYPE shaderType, const std::string& filePath, const InputLayoutInfo* pInputLayoutInfo, InputLayout** ppInputLayout)
{
    std::wstring filePathW(filePath.begin(), filePath.end());
    std::string targetVer = ShaderDX11::getTargetVersion(shaderType);

    ID3DBlob* pCompiledCode = ShaderDX11::compileShader(filePathW.c_str(), targetVer.c_str());
    if (!pCompiledCode) {
        return nullptr;
    }

    if (shaderType == SHADER_TYPE::VERTEX_SHADER && pInputLayoutInfo) {
        *ppInputLayout = InputLayoutDX11::create(pInputLayoutInfo, pCompiledCode, m_pDevice);
    }

    switch (shaderType) {
        case SHADER_TYPE::VERTEX_SHADER:
            return ShaderDX11::createVertexShader(shaderType, pCompiledCode, filePath, m_pDevice);
        case SHADER_TYPE::HULL_SHADER:
            return ShaderDX11::createHullShader(shaderType, pCompiledCode, filePath, m_pDevice);
        case SHADER_TYPE::DOMAIN_SHADER:
            return ShaderDX11::createDomainShader(shaderType, pCompiledCode, filePath, m_pDevice);
        case SHADER_TYPE::GEOMETRY_SHADER:
            return ShaderDX11::createGeometryShader(shaderType, pCompiledCode, filePath, m_pDevice);
        case SHADER_TYPE::FRAGMENT_SHADER:
            return ShaderDX11::createFragmentShader(shaderType, pCompiledCode, filePath, m_pDevice);
        default:
            LOG_ERROR("Erroneous shader type: %d", (int)shaderType);
            return nullptr;
    }
}

std::string DeviceDX11::getShaderPostfixAndExtension(SHADER_TYPE shaderType)
{
    return ShaderDX11::getFilePostfix(shaderType) + ".hlsl";
}
