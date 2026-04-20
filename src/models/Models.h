#pragma once

#include <string>
#include <vector>

namespace ltc
{
    enum class ModuleType
    {
        Tags = 0,
        Policies = 1,
        Audits = 2,
        QapiItems = 3,
        MockSurvey = 4
    };

    struct Record
    {
        int id = 0;
        ModuleType module = ModuleType::Tags;
        std::wstring title;
        std::wstring owner;
        std::wstring status;
        std::wstring dueDate;
        std::wstring priority;
        std::wstring evidenceStatus;
        std::wstring tags;
        std::wstring notes;
    };

    struct AppData
    {
        std::vector<Record> records;
        int nextId = 1;
    };

    inline std::wstring ModuleTypeToDisplayName(ModuleType module)
    {
        switch (module)
        {
        case ModuleType::Tags:
            return L"Tags";
        case ModuleType::Policies:
            return L"Policies";
        case ModuleType::Audits:
            return L"Audits";
        case ModuleType::QapiItems:
            return L"QAPI Items";
        case ModuleType::MockSurvey:
            return L"Mock Survey Prep";
        default:
            return L"Unknown";
        }
    }
}
