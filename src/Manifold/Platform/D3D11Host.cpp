#include "D3D11Host.h"

#pragma comment(lib, "d3d11.lib")

namespace manifold
{
    bool D3D11Host::Create(HWND hwnd)
    {
        DXGI_SWAP_CHAIN_DESC sd{};
        sd.BufferCount = 2;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
#ifdef _DEBUG
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
        D3D_FEATURE_LEVEL featureLevel{};
        const HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
            featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_SwapChain, &m_Device, &featureLevel, &m_Context);
        if (FAILED(res)) return false;
        CreateRenderTarget();
        return true;
    }

    void D3D11Host::CleanupRenderTarget()
    {
        if (m_MainRenderTargetView) { m_MainRenderTargetView->Release(); m_MainRenderTargetView = nullptr; }
    }

    void D3D11Host::CreateRenderTarget()
    {
        ID3D11Texture2D* pBackBuffer = nullptr;
        m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (pBackBuffer)
        {
            m_Device->CreateRenderTargetView(pBackBuffer, nullptr, &m_MainRenderTargetView);
            pBackBuffer->Release();
        }
    }

    void D3D11Host::HandleResize()
    {
        if (m_ResizeWidth == 0 || m_ResizeHeight == 0) return;
        CleanupRenderTarget();
        m_SwapChain->ResizeBuffers(0, m_ResizeWidth, m_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        m_ResizeWidth = m_ResizeHeight = 0;
        CreateRenderTarget();
    }

    void D3D11Host::BeginFrame()
    {
        const float clearColor[4] = { 0.04f, 0.05f, 0.07f, 1.0f };
        m_Context->OMSetRenderTargets(1, &m_MainRenderTargetView, nullptr);
        m_Context->ClearRenderTargetView(m_MainRenderTargetView, clearColor);
    }

    void D3D11Host::EndFrame()
    {
        m_SwapChain->Present(1, 0);
    }

    void D3D11Host::Cleanup()
    {
        CleanupRenderTarget();
        if (m_SwapChain) { m_SwapChain->Release(); m_SwapChain = nullptr; }
        if (m_Context) { m_Context->Release(); m_Context = nullptr; }
        if (m_Device) { m_Device->Release(); m_Device = nullptr; }
    }
}
