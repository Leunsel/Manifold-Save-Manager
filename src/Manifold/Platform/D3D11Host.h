#pragma once

#include <windows.h>
#include <d3d11.h>

namespace manifold
{
    class D3D11Host
    {
    public:
        bool Create(HWND hwnd);
        void Cleanup();
        void BeginFrame();
        void EndFrame();
        void HandleResize();

        ID3D11Device* Device() const { return m_Device; }
        ID3D11DeviceContext* Context() const { return m_Context; }
        void QueueResize(UINT width, UINT height) { m_ResizeWidth = width; m_ResizeHeight = height; }

    private:
        void CleanupRenderTarget();
        void CreateRenderTarget();

    private:
        ID3D11Device* m_Device = nullptr;
        ID3D11DeviceContext* m_Context = nullptr;
        IDXGISwapChain* m_SwapChain = nullptr;
        ID3D11RenderTargetView* m_MainRenderTargetView = nullptr;
        UINT m_ResizeWidth = 0;
        UINT m_ResizeHeight = 0;
    };
}
