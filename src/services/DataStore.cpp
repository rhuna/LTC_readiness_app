#include "DataStore.h"

#include <fstream>
#include <sstream>

namespace ltc
{
    namespace
    {
        std::wstring ReplaceAll(std::wstring value, const std::wstring& from, const std::wstring& to)
        {
            std::size_t startPos = 0;
            while ((startPos = value.find(from, startPos)) != std::wstring::npos)
            {
                value.replace(startPos, from.length(), to);
                startPos += to.length();
            }
            return value;
        }

        std::vector<std::wstring> Split(const std::wstring& line, wchar_t delimiter)
        {
            std::vector<std::wstring> parts;
            std::wstring current;
            bool escaped = false;

            for (wchar_t ch : line)
            {
                if (escaped)
                {
                    current.push_back(ch);
                    escaped = false;
                    continue;
                }

                if (ch == L'\\')
                {
                    escaped = true;
                    current.push_back(ch);
                    continue;
                }

                if (ch == delimiter)
                {
                    parts.push_back(current);
                    current.clear();
                }
                else
                {
                    current.push_back(ch);
                }
            }

            parts.push_back(current);
            return parts;
        }
    }

    DataStore::DataStore(std::filesystem::path filePath)
        : m_filePath(std::move(filePath))
    {
    }

    bool DataStore::Load(AppData& data)
    {
        data.records.clear();
        data.nextId = 1;

        if (!std::filesystem::exists(m_filePath))
        {
            LoadSeedData(data);
            return Save(data);
        }

        std::wifstream input(m_filePath);
        input.imbue(std::locale(""));

        if (!input.is_open())
        {
            return false;
        }

        std::wstring line;
        while (std::getline(input, line))
        {
            if (line.empty())
            {
                continue;
            }

            const auto parts = Split(line, L'|');
            if (parts.size() != 7)
            {
                continue;
            }

            Record record;
            record.id = std::stoi(parts[0]);
            record.module = static_cast<ModuleType>(std::stoi(parts[1]));
            record.title = Unescape(parts[2]);
            record.owner = Unescape(parts[3]);
            record.status = Unescape(parts[4]);
            record.dueDate = Unescape(parts[5]);
            record.tags = Unescape(parts[6]);

            if (!std::getline(input, line))
            {
                break;
            }

            record.notes = Unescape(line);
            data.records.push_back(record);

            if (record.id >= data.nextId)
            {
                data.nextId = record.id + 1;
            }
        }

        if (data.records.empty())
        {
            LoadSeedData(data);
            return Save(data);
        }

        return true;
    }

    bool DataStore::Save(const AppData& data)
    {
        std::filesystem::create_directories(m_filePath.parent_path());

        std::wofstream output(m_filePath, std::ios::trunc);
        output.imbue(std::locale(""));

        if (!output.is_open())
        {
            return false;
        }

        for (const auto& record : data.records)
        {
            output
                << record.id << L"|"
                << static_cast<int>(record.module) << L"|"
                << Escape(record.title) << L"|"
                << Escape(record.owner) << L"|"
                << Escape(record.status) << L"|"
                << Escape(record.dueDate) << L"|"
                << Escape(record.tags) << L"\n";

            output << Escape(record.notes) << L"\n";
        }

        return true;
    }

    void DataStore::LoadSeedData(AppData& data)
    {
        data.records.clear();
        data.nextId = 1;

        auto add = [&](ModuleType module,
                       std::wstring title,
                       std::wstring owner,
                       std::wstring status,
                       std::wstring dueDate,
                       std::wstring tags,
                       std::wstring notes)
        {
            Record record;
            record.id = data.nextId++;
            record.module = module;
            record.title = std::move(title);
            record.owner = std::move(owner);
            record.status = std::move(status);
            record.dueDate = std::move(dueDate);
            record.tags = std::move(tags);
            record.notes = std::move(notes);
            data.records.push_back(std::move(record));
        };

        add(ModuleType::Tags, L"Infection Control", L"DON", L"Active", L"2026-05-01",
            L"F880; isolation; hand hygiene",
            L"Use this tag to group policy reviews, mock survey checks, and audit findings tied to infection prevention.");

        add(ModuleType::Policies, L"Abuse Prevention Policy Review", L"Administrator", L"Needs Review", L"2026-05-10",
            L"abuse; policy; training",
            L"Verify latest revision date, staff acknowledgment, and evidence of annual training completion.");

        add(ModuleType::Audits, L"Medication Cart Storage Audit", L"Unit Manager", L"Open", L"2026-04-28",
            L"med pass; pharmacy; environment",
            L"Check lock compliance, expired meds, unlabeled items, and documentation gaps during random cart rounds.");

        add(ModuleType::QapiItems, L"Falls Reduction Performance Improvement Plan", L"QAPI Chair", L"In Progress", L"2026-05-15",
            L"falls; QAPI; PIP",
            L"Track baseline fall rate, interventions by unit, education actions, and follow-up results for the QAPI committee.");

        add(ModuleType::MockSurvey, L"Resident Interview Readiness Round", L"Social Services", L"Planned", L"2026-04-30",
            L"mock survey; interviews; resident rights",
            L"Prepare residents and staff for likely survey interview areas including dignity, call light response, and choice.");
    }

    const std::filesystem::path& DataStore::GetPath() const noexcept
    {
        return m_filePath;
    }

    std::wstring DataStore::Escape(const std::wstring& value)
    {
        auto escaped = value;
        escaped = ReplaceAll(escaped, L"\\", L"\\\\");
        escaped = ReplaceAll(escaped, L"|", L"\\|");
        escaped = ReplaceAll(escaped, L"\n", L"\\n");
        escaped = ReplaceAll(escaped, L"\r", L"");
        return escaped;
    }

    std::wstring DataStore::Unescape(const std::wstring& value)
    {
        std::wstring result;
        result.reserve(value.size());

        bool escaped = false;
        for (wchar_t ch : value)
        {
            if (escaped)
            {
                if (ch == L'n')
                {
                    result.push_back(L'\n');
                }
                else
                {
                    result.push_back(ch);
                }
                escaped = false;
            }
            else if (ch == L'\\')
            {
                escaped = true;
            }
            else
            {
                result.push_back(ch);
            }
        }

        return result;
    }
}
