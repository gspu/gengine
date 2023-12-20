#include "SaveManager.h"

#include "FileSystem.h"
#include "Paths.h"
#include "PersistState.h"

SaveManager gSaveManager;

SaveManager::SaveManager()
{
    std::string prefsPath = Paths::GetSaveDataPath("Prefs.ini");

    uint32_t bufferSize = 0;
    uint8_t* buffer = File::ReadIntoBuffer(prefsPath, bufferSize);
    mPrefs = new Config("Prefs.ini", AssetScope::Manual);
    mPrefs->Load(buffer, bufferSize);

    // Increment run count.
    int runCount = mPrefs->GetInt("App", "Run Count", 0);
    ++runCount;
    mPrefs->Set("App", "Run Count", runCount);

    // Save changes.
    mPrefs->Save(prefsPath);
}

SaveManager::~SaveManager()
{
    SavePrefs();
    delete mPrefs;
}

void SaveManager::Save(const std::string& saveName)
{
    // Save prefs any time the game is saved.
    SavePrefs();

    // Figure out name for the save file.
    // This depends on number of saves in the save folder.
    std::string saveFilePath = Path::Combine({ Paths::GetSaveDataPath(), "Save Games", "save0001.gk3" });

    PersistState state(saveFilePath.c_str(), PersistState::IOMode::Write, PersistState::Format::Binary);
    //TODO: Write out some stuff.
}

void SaveManager::Load(const std::string& saveName)
{
    PersistState state(saveName.c_str(), PersistState::IOMode::Read, PersistState::Format::Binary);
}

void SaveManager::SavePrefs()
{
    mPrefs->Save(Paths::GetSaveDataPath("Prefs.ini"));
}

int SaveManager::GetRunCount() const
{
    return mPrefs->GetInt("App", "Run Count");
}