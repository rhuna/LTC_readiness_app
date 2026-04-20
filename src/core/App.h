#pragma once

#include "services/DataStore.h"
#include "ui/MainWindow.h"

namespace ltc
{
    class App
    {
    public:
        App();
        int Run();

    private:
        AppData m_data;
        DataStore m_dataStore;
        MainWindow m_mainWindow;
    };
}
