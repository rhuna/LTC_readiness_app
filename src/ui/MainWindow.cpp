#include "MainWindow.h"

#include <commctrl.h>
#include <sstream>
#include <vector>

namespace ltc
{
    namespace
    {
        constexpr wchar_t kWindowClassName[] = L"LTCSurveyReadinessMainWindow";

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
            1280, 760,
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
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->m_hwnd = hwnd;
        }
        else
        {
            self = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
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
                MessageBoxW(hwnd, L"Data saved successfully.", L"Save", MB_OK | MB_ICONINFORMATION);
                return 0;
            case ID_BTN_REFRESH:
                PopulateRecords();
                UpdateDetailView();
                return 0;
            case ID_NAV_LIST:
                if (notifyCode == LBN_SELCHANGE)
                {
                    PopulateRecords();
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

        m_recordList = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            WC_LISTVIEWW, nullptr,
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
            240, 54, 500, 600,
            hwnd, ControlMenu(ID_RECORD_LIST), GetModuleHandle(nullptr), nullptr);

        m_detailBox = CreateWindowExW(
            WS_EX_CLIENTEDGE,
            L"EDIT", nullptr,
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
            750, 54, 500, 600,
            hwnd, ControlMenu(ID_DETAIL_BOX), GetModuleHandle(nullptr), nullptr);

        SendMessage(m_btnAddSeed, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
        SendMessage(m_btnSave, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
        SendMessage(m_btnRefresh, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
        SendMessage(m_navList, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);
        SendMessage(m_detailBox, WM_SETFONT, reinterpret_cast<WPARAM>(defaultFont), TRUE);

        ListView_SetExtendedListViewStyle(m_recordList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

        LVCOLUMNW column{};
        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

        column.pszText = const_cast<LPWSTR>(L"Title");
        column.cx = 220;
        ListView_InsertColumn(m_recordList, 0, reinterpret_cast<const LVCOLUMN*>(&column));

        column.pszText = const_cast<LPWSTR>(L"Owner");
        column.cx = 130;
        ListView_InsertColumn(m_recordList, 1, reinterpret_cast<const LVCOLUMN*>(&column));

        column.pszText = const_cast<LPWSTR>(L"Status");
        column.cx = 110;
        ListView_InsertColumn(m_recordList, 2, reinterpret_cast<const LVCOLUMN*>(&column));

        column.pszText = const_cast<LPWSTR>(L"Due Date");
        column.cx = 100;
        ListView_InsertColumn(m_recordList, 3, reinterpret_cast<const LVCOLUMN*>(&column));
    }

    void MainWindow::LayoutControls(HWND hwnd)
    {
        RECT rect{};
        GetClientRect(hwnd, &rect);

        const int padding = 10;
        const int toolbarHeight = 44;
        const int contentTop = padding + toolbarHeight;
        const int contentHeight = (rect.bottom - rect.top) - contentTop - padding;
        const int navWidth = 220;
        const int detailWidth = 360;
        const int centerWidth = (rect.right - rect.left) - navWidth - detailWidth - (padding * 4);

        MoveWindow(m_btnAddSeed, padding, padding, 140, 32, TRUE);
        MoveWindow(m_btnSave, padding + 150, padding, 100, 32, TRUE);
        MoveWindow(m_btnRefresh, padding + 260, padding, 100, 32, TRUE);

        MoveWindow(m_navList, padding, contentTop, navWidth, contentHeight, TRUE);
        MoveWindow(m_recordList, padding * 2 + navWidth, contentTop, centerWidth, contentHeight, TRUE);
        MoveWindow(m_detailBox, padding * 3 + navWidth + centerWidth, contentTop, detailWidth, contentHeight, TRUE);
    }

    void MainWindow::PopulateNavigation()
    {
        SendMessage(m_navList, LB_RESETCONTENT, 0, 0);

        SendMessage(m_navList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Tags"));
        SendMessage(m_navList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Policies"));
        SendMessage(m_navList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Audits"));
        SendMessage(m_navList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"QAPI Items"));
        SendMessage(m_navList, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Mock Survey Prep"));

        SendMessage(m_navList, LB_SETCURSEL, 0, 0);
    }

    ModuleType MainWindow::GetSelectedModule() const
    {
        const auto selection = static_cast<int>(SendMessage(m_navList, LB_GETCURSEL, 0, 0));
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
            ListView_InsertItem(m_recordList, reinterpret_cast<const LVITEM*>(&item));

            ListView_SetItemText(m_recordList, i, 1, const_cast<LPWSTR>(record->owner.c_str()));
            ListView_SetItemText(m_recordList, i, 2, const_cast<LPWSTR>(record->status.c_str()));
            ListView_SetItemText(m_recordList, i, 3, const_cast<LPWSTR>(record->dueDate.c_str()));
        }

        if (!filtered.empty())
        {
            ListView_SetItemState(m_recordList, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        }
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

    std::wstring MainWindow::BuildDetailText(const Record& record) const
    {
        std::wstringstream builder;
        builder << L"Module: " << ModuleTypeToDisplayName(record.module) << L"\r\n\r\n";
        builder << L"Title: " << record.title << L"\r\n";
        builder << L"Owner: " << record.owner << L"\r\n";
        builder << L"Status: " << record.status << L"\r\n";
        builder << L"Due Date: " << record.dueDate << L"\r\n";
        builder << L"Tags: " << record.tags << L"\r\n";
        builder << L"Record ID: " << record.id << L"\r\n\r\n";
        builder << L"Notes:\r\n" << record.notes << L"\r\n\r\n";
        builder << L"Storage Path:\r\n" << m_dataStore->GetPath().wstring();
        return builder.str();
    }

    void MainWindow::AddQuickSeedItem()
    {
        Record record;
        record.id = m_data->nextId++;
        record.module = GetSelectedModule();
        record.title = L"New Readiness Item";
        record.owner = L"Assigned Owner";
        record.status = L"Open";
        record.dueDate = L"2026-05-31";
        record.tags = L"survey; follow-up";
        record.notes = L"This is a quick starter item added from the desktop app. Replace it with a real edit dialog in the next version.";

        m_data->records.push_back(std::move(record));
        PopulateRecords();
        UpdateDetailView();
    }
}
