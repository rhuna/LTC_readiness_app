#include "App.h"

#include <windows.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

namespace ltc
{
    App::App()
        : m_dataStore(L"data/ltc_survey_readiness.dbtxt")
    {
    }

    int App::Run()
    {
        INITCOMMONCONTROLSEX icex{};
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_WIN95_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES;
        InitCommonControlsEx(&icex);

        if (!m_dataStore.Load(m_data))
        {
            m_dataStore.LoadSeedData(m_data);
            m_dataStore.Save(m_data);
        }

        if (!m_mainWindow.Create(m_data, m_dataStore))
        {
            return -1;
        }

        MSG message{};
        while (GetMessage(&message, nullptr, 0, 0) > 0)
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }

        return static_cast<int>(message.wParam);
    }
}
