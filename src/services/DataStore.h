#pragma once

#include "models/Models.h"
#include <filesystem>
#include <string>

namespace ltc
{
    class DataStore
    {
    public:
        explicit DataStore(std::filesystem::path filePath);

        bool Load(AppData& data);
        bool Save(const AppData& data);
        void LoadSeedData(AppData& data);

        const std::filesystem::path& GetPath() const noexcept;

    private:
        std::filesystem::path m_filePath;

        static std::wstring Escape(const std::wstring& value);
        static std::wstring Unescape(const std::wstring& value);
    };
}
