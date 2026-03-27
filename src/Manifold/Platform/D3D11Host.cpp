#include "D3D11Host.hpp"

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
        if (!m_SwapChain || !m_Device) return;
        ID3D11Texture2D* backBuffer = nullptr;
        const HRESULT hr = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr) || !backBuffer) return;
        m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_MainRenderTargetView);
        backBuffer->Release();
    }

    void D3D11Host::HandleResize()
    {
        if (m_ResizeWidth == 0 || m_ResizeHeight == 0) return;
        if (!m_Device || !m_Context || !m_SwapChain) return;
        m_Context->OMSetRenderTargets(0, nullptr, nullptr);
        m_Context->ClearState();
        m_Context->Flush();
        CleanupRenderTarget();
        const UINT width = m_ResizeWidth;
        const UINT height = m_ResizeHeight;
        m_ResizeWidth = 0;
        m_ResizeHeight = 0;
        const HRESULT hr = m_SwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
        if (FAILED(hr)) return;
        CreateRenderTarget();
    }

    void D3D11Host::BeginFrame()
    {
        if (!m_Context || !m_MainRenderTargetView) return;
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
