# Long-Term Care Survey Readiness App

A native C++ Windows desktop application for organizing long-term care survey readiness work in one place.

## What it does

This starter repo gives you a Windows application that tracks:

- Tags and focus areas
- Policies and review dates
- Audit tasks and owners
- QAPI items and follow-up status
- Mock survey preparation items
- Priority and evidence readiness for each item

It is intentionally designed as a practical foundation you can keep extending.

## Stack

- C++17
- Win32 desktop UI
- CMake
- Local file persistence with a plain text data file

## Why this architecture

This version avoids heavy external dependencies so it is easier to open, build, and evolve on Windows.  
The app is split into:

- `models` for the core data structures
- `services` for persistence and seed data
- `ui` for the Windows interface
- `core` for app startup and message loop

## Build on Windows with Visual Studio 2022

Open a Developer Command Prompt and run:

```bat
cd C:\dev
git clone <your-repo-url> LTCSurveyReadinessApp
cd LTCSurveyReadinessApp
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
build\Release\LTCSurveyReadinessApp.exe
```

## Build on Windows with MinGW

```bat
cmake -S . -B build -G "MinGW Makefiles" ^
  -DCMAKE_CXX_COMPILER=C:/msys64/ucrt64/bin/g++.exe
cmake --build build
build\LTCSurveyReadinessApp.exe
```

## Current behavior

The app loads seeded survey-readiness content the first time it starts.
It then saves to:

```text
data\ltc_survey_readiness.dbtxt
```

The current UI includes:

- A left navigation list for modules
- A center list showing records for the selected module
- A right detail panel showing the selected record
- Toolbar buttons to add seed data, save, and refresh
- A dashboard summary panel with readiness indicators
- Per-record priority and evidence status
- Overdue tracking based on due dates

## v4 additions

- Added `priority` and `evidenceStatus` to the record model
- Expanded seeded data with more realistic survey-readiness items
- Added backward-compatible loading for older data files
- Upgraded the record grid to show priority and evidence columns
- Added estimated readiness percentage in the summary panel
- Added overdue and high-priority counts in the summary panel
- Added overdue visibility in the detail panel
- Updated quick-add items so they include priority and evidence defaults

## Next upgrades I would build on top of this

- True add/edit dialogs for each module
- Search and filtering by tag, owner, due date, and status
- Dashboard metrics with color-coded risk indicators
- Evidence/document attachment tracking
- Survey deficiency drill-down
- Calendar reminders
- Export to CSV or PDF
- Role-based users and sign-off workflows
- SQLite storage instead of flat-file persistence
- Rich mock survey workflow with sections, findings, and action plans

## Repo contents

```text
LTCSurveyReadinessApp/
  CMakeLists.txt
  README.md
  build_release.bat
  src/
    main.cpp
    core/
    models/
    services/
    ui/
  data/
```

## Notes

This repo snapshot was generated here and not compiled in a Windows toolchain inside this environment, so you should treat it as a clean starter repo snapshot. The structure and code are written to be build-ready for a normal Windows C++ setup.
