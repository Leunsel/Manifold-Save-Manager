#pragma once

#include <memory>
#include <windows.h>

namespace manifold
{
    class SaveManagerApp
    {
    public:
        explicit SaveManagerApp(HWND hwnd);
        ~SaveManagerApp();

        SaveManagerApp(const SaveManagerApp&) = delete;
        SaveManagerApp& operator=(const SaveManagerApp&) = delete;

        SaveManagerApp(SaveManagerApp&&) noexcept;
        SaveManagerApp& operator=(SaveManagerApp&&) noexcept;

        void Render();

    private:
        struct Impl;
        std::unique_ptr<Impl> m_Impl;
    };
}