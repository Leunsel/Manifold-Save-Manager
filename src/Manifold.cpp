#include <windows.h>
#include <windowsx.h>
#include <tchar.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "Manifold/Platform/D3D11Host.hpp"
#include "Manifold/UI/SaveManagerApp.hpp"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

static void ApplyBorderlessDwmStyling(HWND hwnd)
{
    if (!hwnd) return;
    constexpr COLORREF kDwmColorNone = 0xFFFFFFFE;
    DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &kDwmColorNone, sizeof(kDwmColorNone));
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED );
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace manifold;

static D3D11Host g_D3DHost;
static bool g_InSizeMove = false;

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return TRUE;

    switch (msg)
    {
        case WM_NCCALCSIZE:
        {
            if (wParam == TRUE) return 0;
            break;
        }

        case WM_GETMINMAXINFO:
        {
            MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
            if (!mmi) break;

            mmi->ptMinTrackSize.x = 1200;
            mmi->ptMinTrackSize.y = 720;

            HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
            if (monitor)
            {
                MONITORINFO mi{};
                mi.cbSize = sizeof(mi);

                if (GetMonitorInfoW(monitor, &mi))
                {
                    const RECT& work = mi.rcWork;
                    const RECT& monitorRect = mi.rcMonitor;

                    mmi->ptMaxPosition.x = work.left - monitorRect.left;
                    mmi->ptMaxPosition.y = work.top - monitorRect.top;
                    mmi->ptMaxSize.x = work.right - work.left;
                    mmi->ptMaxSize.y = work.bottom - work.top;
                    mmi->ptMaxTrackSize.x = mmi->ptMaxSize.x;
                    mmi->ptMaxTrackSize.y = mmi->ptMaxSize.y;
                }
            }
            return 0;
        }

        case WM_NCHITTEST:
        {
            if (IsZoomed(hWnd))
                return HTCLIENT;

            LRESULT hit = DefWindowProcW(hWnd, msg, wParam, lParam);
            if (hit != HTCLIENT) return hit;

            RECT rc{};
            if (!GetWindowRect(hWnd, &rc)) return HTCLIENT;

            const POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            const int border = 8;

            const bool left = pt.x >= rc.left && pt.x < rc.left + border;
            const bool right = pt.x < rc.right && pt.x >= rc.right - border;
            const bool top = pt.y >= rc.top && pt.y < rc.top + border;
            const bool bottom = pt.y < rc.bottom && pt.y >= rc.bottom - border;

            if (top && left) return HTTOPLEFT;
            if (top && right) return HTTOPRIGHT;
            if (bottom && left) return HTBOTTOMLEFT;
            if (bottom && right) return HTBOTTOMRIGHT;
            if (left) return HTLEFT;
            if (right) return HTRIGHT;
            if (top) return HTTOP;
            if (bottom) return HTBOTTOM;

            return HTCLIENT;
        }

        case WM_ENTERSIZEMOVE:
        {
            g_InSizeMove = true;
            return 0;
        }

        case WM_EXITSIZEMOVE:
        {
            g_InSizeMove = false;

            RECT rc{};
            if (GetClientRect(hWnd, &rc))
            {
                const UINT width = static_cast<UINT>(rc.right - rc.left);
                const UINT height = static_cast<UINT>(rc.bottom - rc.top);
                if (width > 0 && height > 0) g_D3DHost.QueueResize(width, height);
            }

            InvalidateRect(hWnd, nullptr, FALSE);
            return 0;
        }

        case WM_SIZE:
        {
            if (g_D3DHost.Device() != nullptr && wParam != SIZE_MINIMIZED)
            {
                const UINT width = static_cast<UINT>(LOWORD(lParam));
                const UINT height = static_cast<UINT>(HIWORD(lParam));

                if (width > 0 && height > 0)
                {
                    g_D3DHost.QueueResize(width, height);
                }

                InvalidateRect(hWnd, nullptr, FALSE);
            }
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps{};
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
        {
            return 1;
        }

        case WM_SYSCOMMAND:
        {
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        }

        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd)
{
    const HRESULT coResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool coInitialized = SUCCEEDED(coResult);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ManifoldSaveManagerWindowClass";

    if (!RegisterClassExW(&wc))
    {
        if (coInitialized) CoUninitialize();
        return 1;
    }

    const DWORD style = WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU;
    const DWORD exStyle = WS_EX_APPWINDOW;
    HWND hwnd = CreateWindowExW(exStyle, wc.lpszClassName, L"Manifold Save Manager", style, 100, 100, 1700, 960, nullptr, nullptr, wc.hInstance, nullptr);

    if (!hwnd)
    {
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        if (coInitialized)
            CoUninitialize();
        return 1;
    }

    ApplyBorderlessDwmStyling(hwnd);

    if (!g_D3DHost.Create(hwnd))
    {
        g_D3DHost.Cleanup();
        DestroyWindow(hwnd);
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        if (coInitialized) CoUninitialize();
        return 1;
    }

    ShowWindow(hwnd, nShowCmd ? nShowCmd : SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_D3DHost.Device(), g_D3DHost.Context());

    SaveManagerApp app(hwnd);

    bool done = false;
    while (!done)
    {
        MSG msg{};
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                done = true;
                break;
            }
        }

        if (done) break;

        g_D3DHost.HandleResize();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        app.Render();

        ImGui::Render();

        g_D3DHost.BeginFrame();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_D3DHost.EndFrame();

        if (g_InSizeMove)
        {
            InvalidateRect(hwnd, nullptr, FALSE);
        }
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    g_D3DHost.Cleanup();

    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    if (coInitialized) CoUninitialize();

    return 0;
}