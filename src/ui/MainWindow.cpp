#include "MainWindow.h"

#include <commctrl.h>
#include <sstream>
#include <vector>

namespace ltc
{
    namespace
    {
        constexpr wchar_t kWindowClassName[] = L"LTCSurveyReadinessMainWindow";
        constexpr wchar_t kTodayDate[] = L"2026-04-20";

        inline HMENU ControlMenu(int id)
        {
            return reinterpret_cast<HMENU>(static_cast<intptr_t>(id));
        }

        std::vector<const Record*> FilterByModule(const AppData& data, ModuleType module)
        {
            std::vector<const Record*> filtered;
            for (const auto& record : data.records)
            {
                if (record.module == module)
                {
                    filtered.push_back(&record);
                }
            }
            return filtered;
        }
    }

    bool MainWindow::Create(AppData& data, DataStore& dataStore)
    {
        m_data = &data;
        m_dataStore = &dataStore;

        HINSTANCE instance = GetModuleHandle(nullptr);

        INITCOMMONCONTROLSEX icex{};
        icex.dwSize = sizeof(icex);
        icex.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&icex);

        WNDCLASSW wc{};
        wc.lpfnWndProc = MainWindow::WindowProc;
        wc.hInstance = instance;
        wc.lpszClassName = kWindowClassName;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

        RegisterClassW(&wc);

        m_hwnd = CreateWindowExW(
            0,
            kWindowClassName,
            L"Long-Term Care Survey Readiness App",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            1320, 780,
            nullptr,
            nullptr,
            instance,
            this);

        return m_hwnd != nullptr;
    }

    LRESULT CALLBACK MainWindow::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        MainWindow* self = nullptr;

        if (message == WM_NCCREATE)
        {
            auto* createStruct = reinterpret_cast<CREATESTRUCTW*>(lParam);
            self = static_cast<MainWindow*>(createStruct->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->m_hwnd = hwnd;
        }
        else
        {
            self = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        if (self != nullptr)
        {
            return self->HandleMessage(hwnd, message, wParam, lParam);
        }

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    LRESULT MainWindow::HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_CREATE:
            CreateChildControls(hwnd);
            PopulateNavigation();
            PopulateRecords();
            UpdateSummaryView();
            UpdateWindowTitle();
            UpdateDetailView();
            return 0;

        case WM_SIZE:
            LayoutControls(hwnd);
            return 0;

        case WM_COMMAND:
        {
            const int controlId = LOWORD(wParam);
            const int notifyCode = HIWORD(wParam);

            switch (controlId)
            {
            case ID_BTN_ADD_SEED:
                AddQuickSeedItem();
                return 0;
            case ID_BTN_SAVE:
                m_dataStore->Save(*m_data);
                UpdateSummaryView();
                UpdateWindowTitle();
                MessageBoxW(hwnd, L"Data saved successfully.", L"Save", MB_OK | MB_ICONINFORMATION);
                return 0;
            case ID_BTN_REFRESH:
                PopulateRecords();
                UpdateSummaryView();
                UpdateWindowTitle();
                UpdateDetailView();
                return 0;
            case ID_NAV_LIST:
                if (notifyCode == LBN_SELCHANGE)
                {
                    PopulateRecords();
                    UpdateSummaryView();
                    UpdateWindowTitle();
                    UpdateDetailView();
                    return 0;
                }
                break;
            default:
                break;
            }
            break;
        }

        case WM_NOTIFY:
        {
            auto* header = reinterpret_cast<LPNMHDR>(lParam);
            if (header->idFrom == ID_RECORD_LIST && header->code == LVN_ITEMCHANGED)
            {
                UpdateDetailView();
                return 0;
            }
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    void MainWindow::CreateChildControls(HWND hwnd)
    {
        HFONT defaultFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

        m_btnAddSeed = CreateWindowW(
            L"BUTTON", L"Add Sample Item",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 10, 140, 32,
            hwnd, ControlMenu(ID_BTN_ADD_SEED), GetModuleHandle(nullptr), nullptr);

        m_btnSave = CreateWindowW(
            L"BUTTON", L"Save",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            160, 10, 100, 32,
            hwnd, ControlMenu(ID_BTN_SAVE), GetModuleHandle(nullptr), nullptr);

        m_btnRefresh = CreateWindowW(
            L"BUTTON", L"Refresh",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            270, 10, 100, 32,
            hwnd, ControlMenu(ID_BTN_REFRESH), GetModuleHandle(nullptr), nullptr);

        m_navList = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"LISTBOX", nullptr,
            WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL,
            10, 54, 220, 600,
            hwnd, ControlMenu(ID_NAV_LIST), GetModuleHandle(nullptr), nullptr);

        m_summaryBox = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"EDIT", nullptr,
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
            380, 10, 930, 92,
            hwnd, ControlMenu(ID_SUMMARY_BOX), GetModuleHandle(nullptr), nullptr);

        m_recordList = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            WC_LISTVIEWW, nullptr,
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
            240, 54, 620, 600,
            hwnd, ControlMenu(ID_RECORD_LIST), GetModuleHandle(nullptr), nullptr);

        m_detailBox = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"EDIT", nullptr,
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
            870, 54, 440, 600,
            hwnd, ControlMenu(ID_DETAIL_BOX), GetModuleHandle(nullptr), nullptr);

        SendMessageW(m_btnAddSeed, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
        SendMessageW(m_btnSave, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
        SendMessageW(m_btnRefresh, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
        SendMessageW(m_navList, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
        SendMessageW(m_recordList, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
        SendMessageW(m_detailBox, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
        SendMessageW(m_summaryBox, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);

        ListView_SetExtendedListViewStyle(m_recordList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        LVCOLUMNW column{};
        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

        column.pszText = const_cast<LPWSTR>(L"Title");
        column.cx = 210;
        ListView_InsertColumn(m_recordList, 0, &column);

        column.pszText = const_cast<LPWSTR>(L"Owner");
        column.cx = 110;
        ListView_InsertColumn(m_recordList, 1, &column);

        column.pszText = const_cast<LPWSTR>(L"Status");
        column.cx = 95;
        ListView_InsertColumn(m_recordList, 2, &column);

        column.pszText = const_cast<LPWSTR>(L"Due Date");
        column.cx = 92;
        ListView_InsertColumn(m_recordList, 3, &column);

        column.pszText = const_cast<LPWSTR>(L"Priority");
        column.cx = 75;
        ListView_InsertColumn(m_recordList, 4, &column);

        column.pszText = const_cast<LPWSTR>(L"Evidence");
        column.cx = 110;
        ListView_InsertColumn(m_recordList, 5, &column);
    }

    void MainWindow::LayoutControls(HWND hwnd)
    {
        RECT rect{};
        GetClientRect(hwnd, &rect);

        const int padding = 10;
        const int toolbarHeight = 44;
        const int summaryHeight = 92;
        const int contentTop = padding + toolbarHeight + summaryHeight + padding;
        const int contentHeight = (rect.bottom - rect.top) - contentTop - padding;
        const int navWidth = 220;
        const int detailWidth = 440;
        const int centerWidth = (rect.right - rect.left) - navWidth - detailWidth - (padding * 4);

        MoveWindow(m_btnAddSeed, padding, padding, 140, 32, TRUE);
        MoveWindow(m_btnSave, padding + 150, padding, 100, 32, TRUE);
        MoveWindow(m_btnRefresh, padding + 260, padding, 100, 32, TRUE);

        MoveWindow(m_summaryBox, padding + 370, padding, (rect.right - rect.left) - (padding * 2) - 370, summaryHeight, TRUE);

        MoveWindow(m_navList, padding, contentTop, navWidth, contentHeight, TRUE);
        MoveWindow(m_recordList, padding * 2 + navWidth, contentTop, centerWidth, contentHeight, TRUE);
        MoveWindow(m_detailBox, padding * 3 + navWidth + centerWidth, contentTop, detailWidth, contentHeight, TRUE);
    }

    void MainWindow::PopulateNavigation()
    {
        SendMessageW(m_navList, LB_RESETCONTENT, 0, 0);

        SendMessageW(m_navList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Tags"));
        SendMessageW(m_navList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Policies"));
        SendMessageW(m_navList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Audits"));
        SendMessageW(m_navList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"QAPI Items"));
        SendMessageW(m_navList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Mock Survey Prep"));

        SendMessageW(m_navList, LB_SETCURSEL, 0, 0);
    }

    ModuleType MainWindow::GetSelectedModule() const
    {
        const auto selection = static_cast<int>(SendMessageW(m_navList, LB_GETCURSEL, 0, 0));
        switch (selection)
        {
        case 0: return ModuleType::Tags;
        case 1: return ModuleType::Policies;
        case 2: return ModuleType::Audits;
        case 3: return ModuleType::QapiItems;
        case 4: return ModuleType::MockSurvey;
        default: return ModuleType::Tags;
        }
    }

    void MainWindow::PopulateRecords()
    {
        ListView_DeleteAllItems(m_recordList);

        const auto filtered = FilterByModule(*m_data, GetSelectedModule());

        for (int i = 0; i < static_cast<int>(filtered.size()); ++i)
        {
            const auto* record = filtered[static_cast<std::size_t>(i)];

            LVITEMW item{};
            item.mask = LVIF_TEXT;
            item.iItem = i;
            item.iSubItem = 0;
            item.pszText = const_cast<LPWSTR>(record->title.c_str());
            ListView_InsertItem(m_recordList, &item);

            ListView_SetItemText(m_recordList, i, 1, const_cast<LPWSTR>(record->owner.c_str()));
            ListView_SetItemText(m_recordList, i, 2, const_cast<LPWSTR>(record->status.c_str()));
            ListView_SetItemText(m_recordList, i, 3, const_cast<LPWSTR>(record->dueDate.c_str()));
            ListView_SetItemText(m_recordList, i, 4, const_cast<LPWSTR>(record->priority.c_str()));
            ListView_SetItemText(m_recordList, i, 5, const_cast<LPWSTR>(record->evidenceStatus.c_str()));
        }

        if (!filtered.empty())
        {
            ListView_SetItemState(m_recordList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        }

        UpdateSummaryView();
        UpdateWindowTitle();
    }

    int MainWindow::GetSelectedRecordIndexForModule() const
    {
        return ListView_GetNextItem(m_recordList, -1, LVNI_SELECTED);
    }

    void MainWindow::UpdateDetailView()
    {
        const auto filtered = FilterByModule(*m_data, GetSelectedModule());
        const int selectedIndex = GetSelectedRecordIndexForModule();

        if (selectedIndex < 0 || selectedIndex >= static_cast<int>(filtered.size()))
        {
            SetWindowTextW(m_detailBox, L"Select an item to view its survey-readiness details.");
            return;
        }

        const auto* record = filtered[static_cast<std::size_t>(selectedIndex)];
        const auto text = BuildDetailText(*record);
        SetWindowTextW(m_detailBox, text.c_str());
    }

    void MainWindow::UpdateSummaryView()
    {
        const auto text = BuildSummaryText();
        SetWindowTextW(m_summaryBox, text.c_str());
    }

    void MainWindow::UpdateWindowTitle()
    {
        std::wstringstream title;
        title << L"Long-Term Care Survey Readiness App v4 - " << ModuleTypeToDisplayName(GetSelectedModule())
              << L" (" << m_data->records.size() << L" total items)";
        SetWindowTextW(m_hwnd, title.str().c_str());
    }

    bool MainWindow::IsOverdue(const Record& record) const
    {
        return !record.dueDate.empty() && record.dueDate < kTodayDate &&
               record.status != L"Closed" && record.status != L"Completed";
    }

    bool MainWindow::IsHighPriority(const Record& record) const
    {
        return record.priority == L"High" || record.priority == L"Critical";
    }

    int MainWindow::CalculateReadinessPercent(const std::vector<const Record*>& filtered) const
    {
        if (filtered.empty())
        {
            return 100;
        }

        int score = 0;
        const int maxScore = static_cast<int>(filtered.size()) * 4;

        for (const auto* record : filtered)
        {
            if (record->status == L"Completed" || record->status == L"Closed" || record->status == L"Active")
            {
                score += 2;
            }
            else if (record->status == L"In Progress")
            {
                score += 1;
            }

            if (record->evidenceStatus == L"On File")
            {
                score += 2;
            }
            else if (record->evidenceStatus == L"Partial")
            {
                score += 1;
            }
        }

        return (score * 100) / maxScore;
    }

    std::wstring MainWindow::BuildDetailText(const Record& record) const
    {
        std::wstringstream builder;
        builder << L"Module: " << ModuleTypeToDisplayName(record.module) << L"\r\n\r\n";
        builder << L"Title: " << record.title << L"\r\n";
        builder << L"Owner: " << record.owner << L"\r\n";
        builder << L"Status: " << record.status << L"\r\n";
        builder << L"Due Date: " << record.dueDate << L"\r\n";
        builder << L"Priority: " << record.priority << L"\r\n";
        builder << L"Evidence Status: " << record.evidenceStatus << L"\r\n";
        builder << L"Overdue: " << (IsOverdue(record) ? L"Yes" : L"No") << L"\r\n";
        builder << L"Tags: " << record.tags << L"\r\n";
        builder << L"Record ID: " << record.id << L"\r\n\r\n";
        builder << L"Notes:\r\n" << record.notes << L"\r\n\r\n";
        builder << L"Storage Path:\r\n" << m_dataStore->GetPath().wstring();
        return builder.str();
    }

    std::wstring MainWindow::BuildSummaryText() const
    {
        const ModuleType module = GetSelectedModule();
        const auto filtered = FilterByModule(*m_data, module);

        int openCount = 0;
        int inProgressCount = 0;
        int reviewCount = 0;
        int plannedCount = 0;
        int otherCount = 0;
        int overdueCount = 0;
        int highPriorityCount = 0;
        int evidenceReadyCount = 0;

        for (const auto* record : filtered)
        {
            const std::wstring& status = record->status;
            if (status == L"Open" || status == L"Active")
            {
                ++openCount;
            }
            else if (status == L"In Progress")
            {
                ++inProgressCount;
            }
            else if (status == L"Needs Review")
            {
                ++reviewCount;
            }
            else if (status == L"Planned")
            {
                ++plannedCount;
            }
            else
            {
                ++otherCount;
            }

            if (IsOverdue(*record))
            {
                ++overdueCount;
            }

            if (IsHighPriority(*record))
            {
                ++highPriorityCount;
            }

            if (record->evidenceStatus == L"On File")
            {
                ++evidenceReadyCount;
            }
        }

        std::wstringstream builder;
        builder << L"Dashboard Summary\r\n";
        builder << L"Current Module: " << ModuleTypeToDisplayName(module)
                << L" | Visible Items: " << filtered.size()
                << L" | Estimated Readiness: " << CalculateReadinessPercent(filtered) << L"%\r\n";
        builder << L"Open/Active: " << openCount
                << L" | In Progress: " << inProgressCount
                << L" | Needs Review: " << reviewCount
                << L" | Planned: " << plannedCount
                << L" | Other: " << otherCount << L"\r\n";
        builder << L"High Priority: " << highPriorityCount
                << L" | Overdue: " << overdueCount
                << L" | Evidence On File: " << evidenceReadyCount
                << L" | Today Reference: " << kTodayDate << L"\r\n";
        builder << L"Tip: Focus first on overdue items, high-priority gaps, and records that still need evidence collection before survey week.";
        return builder.str();
    }

    void MainWindow::AddQuickSeedItem()
    {
        Record record;
        record.id = m_data->nextId++;
        record.module = GetSelectedModule();

        switch (record.module)
        {
        case ModuleType::Tags:
            record.title = L"New Survey Tag Group";
            record.owner = L"DNS";
            record.status = L"Active";
            record.dueDate = L"2026-05-31";
            record.priority = L"Medium";
            record.evidenceStatus = L"On File";
            record.tags = L"tag; survey focus";
            record.notes = L"Group related preparation work under a reusable readiness tag so audits, policy reviews, and mock survey tasks stay linked together.";
            break;
        case ModuleType::Policies:
            record.title = L"New Policy Review Item";
            record.owner = L"Administrator";
            record.status = L"Needs Review";
            record.dueDate = L"2026-05-31";
            record.priority = L"High";
            record.evidenceStatus = L"Needs Collection";
            record.tags = L"policy; annual review";
            record.notes = L"Use this item for a policy document that needs revision checks, staff acknowledgment verification, or supporting evidence for survey readiness.";
            break;
        case ModuleType::Audits:
            record.title = L"New Audit Follow-Up";
            record.owner = L"Unit Manager";
            record.status = L"Open";
            record.dueDate = L"2026-05-31";
            record.priority = L"High";
            record.evidenceStatus = L"Partial";
            record.tags = L"audit; observation";
            record.notes = L"Capture audit scope, observations, corrective action, and validation steps so the team can show consistent readiness work.";
            break;
        case ModuleType::QapiItems:
            record.title = L"New QAPI Action Item";
            record.owner = L"QAPI Chair";
            record.status = L"In Progress";
            record.dueDate = L"2026-05-31";
            record.priority = L"High";
            record.evidenceStatus = L"Partial";
            record.tags = L"QAPI; action plan";
            record.notes = L"Track issue statement, root cause, interventions, assigned leads, and follow-up measures for the next QAPI meeting.";
            break;
        case ModuleType::MockSurvey:
            record.title = L"New Mock Survey Prep Task";
            record.owner = L"Interdisciplinary Team";
            record.status = L"Planned";
            record.dueDate = L"2026-05-31";
            record.priority = L"Medium";
            record.evidenceStatus = L"Needs Collection";
            record.tags = L"mock survey; tracer";
            record.notes = L"Use this prep task for resident interview rounds, kitchen tracers, med pass observations, or environment-of-care checks.";
            break;
        }

        m_data->records.push_back(std::move(record));
        PopulateRecords();
        UpdateSummaryView();
        UpdateWindowTitle();
        UpdateDetailView();
    }
}
