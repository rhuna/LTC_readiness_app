#pragma once

#include "models/Models.h"
#include "services/DataStore.h"

#include <windows.h>

namespace ltc
{
    class MainWindow
    {
    public:
        MainWindow() = default;
        bool Create(AppData& data, DataStore& dataStore);

    private:
        static constexpr int ID_NAV_LIST = 101;
        static constexpr int ID_RECORD_LIST = 102;
        static constexpr int ID_DETAIL_BOX = 103;
        static constexpr int ID_BTN_ADD_SEED = 104;
        static constexpr int ID_BTN_SAVE = 105;
        static constexpr int ID_BTN_REFRESH = 106;
        static constexpr int ID_SUMMARY_BOX = 107;

        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
        LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

        void CreateChildControls(HWND hwnd);
        void LayoutControls(HWND hwnd);
        void PopulateNavigation();
        void PopulateRecords();
        void UpdateSummaryView();
        void UpdateWindowTitle();
        void UpdateDetailView();
        void AddQuickSeedItem();
        ModuleType GetSelectedModule() const;
        int GetSelectedRecordIndexForModule() const;
        std::wstring BuildDetailText(const Record& record) const;
        std::wstring BuildSummaryText() const;
        bool IsOverdue(const Record& record) const;
        bool IsHighPriority(const Record& record) const;
        int CalculateReadinessPercent(const std::vector<const Record*>& filtered) const;

        HWND m_hwnd = nullptr;
        HWND m_navList = nullptr;
        HWND m_recordList = nullptr;
        HWND m_detailBox = nullptr;
        HWND m_summaryBox = nullptr;
        HWND m_btnAddSeed = nullptr;
        HWND m_btnSave = nullptr;
        HWND m_btnRefresh = nullptr;

        AppData* m_data = nullptr;
        DataStore* m_dataStore = nullptr;
    };
}
