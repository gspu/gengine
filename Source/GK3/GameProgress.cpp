#include "GameProgress.h"

#include "AssetManager.h"
#include "Config.h"
#include "GMath.h"
#include "IniParser.h"
#include "Localizer.h"
#include "ReportManager.h"
#include "SceneManager.h"
#include "StatusOverlay.h"
#include "StringUtil.h"
#include "TextAsset.h"

GameProgress gGameProgress;

void GameProgress::Init()
{
    // Parse valid score events (and score amount) into map of score events.
    TextAsset* textFile = gAssetManager.LoadText("Scores.txt");
    IniParser parser(textFile->GetText(), textFile->GetTextLength());
    IniSection section;
    while(parser.ReadNextSection(section))
    {
        for(auto& line : section.lines)
        {
            for(auto& entry : line.entries)
            {
                int score = entry.GetValueAsInt();
                mScoreEvents[entry.key] = score;
            }
        }
    }

    // Load max score from config file.
    Config* config = gAssetManager.LoadConfig("GAME.CFG");
    if(config != nullptr)
    {
        mMaxScore = config->GetInt("Logic", "Max Score", mMaxScore);
    }
}

void GameProgress::SetScore(int score)
{
	mScore = Math::Clamp(score, 0, mMaxScore);
}

void GameProgress::IncreaseScore(int points)
{
	SetScore(mScore + points);
}

void GameProgress::ChangeScore(const std::string& scoreName)
{
    // Make sure it is a valid score name.
    auto validEventsIt = mScoreEvents.find(scoreName);
    if(validEventsIt == mScoreEvents.end())
    {
        gReportManager.Log("Error", StringUtil::Format("Illegal score name (%s)", scoreName.c_str()));
        return;
    }

    // If we haven't already gotten this score event, we can now get it.
    auto achievedEventsIt = mScoreEventFlags.find(scoreName);
    if(achievedEventsIt == mScoreEventFlags.end())
    {
        // Flag that we've achieved this one.
        mScoreEventFlags[scoreName] = true;

        // Give the points.
        IncreaseScore(validEventsIt->second);

        // Refresh status overlay to show updated point count.
        Scene* scene = gSceneManager.GetScene();
        if(scene != nullptr)
        {
            StatusOverlay* statusOverlay = scene->GetStatusOverlay();
            if(statusOverlay != nullptr)
            {
                statusOverlay->Refresh();
            }
        }
    }
}

void GameProgress::SetTimeblock(const Timeblock& timeblock)
{
	mLastTimeblock = mTimeblock;
	mTimeblock = timeblock;
	
	// Chat counts are reset on time block change.
	mChatCounts.clear();
}

std::string GameProgress::GetTimeblockDisplayName() const
{
    return GetTimeblockDisplayName(mTimeblock.ToString());
}

std::string GameProgress::GetTimeblockDisplayName(const std::string& timeblockStr) const
{
    // Keys for timeblocks are in form "Day110A".
    return gLocalizer.GetText("Day" + mTimeblock.ToString());
}

int GameProgress::GetGameVariable(const std::string& varName) const
{
	auto it = mGameVariables.find(varName);
	if(it != mGameVariables.end())
	{
		return it->second;
	}
	return 0;
}

void GameProgress::SetGameVariable(const std::string& varName, int value)
{
	mGameVariables[varName] = value;
}

void GameProgress::IncGameVariable(const std::string& varName)
{
	++mGameVariables[varName];
}

int GameProgress::GetChatCount(const std::string& noun) const
{
	auto it = mChatCounts.find(noun);
	if(it != mChatCounts.end())
	{
		return it->second;
	}
	return 0;
}

void GameProgress::SetChatCount(const std::string& noun, int count)
{
	mChatCounts[noun] = count;
}

void GameProgress::IncChatCount(const std::string& noun)
{
	++mChatCounts[noun];
}

int GameProgress::GetTopicCount(const std::string& noun, const std::string& topic) const
{
    return GetTopicCount(Scene::GetEgoName(), noun, topic);
}

int GameProgress::GetTopicCount(const std::string& actor, const std::string& noun, const std::string& topic) const
{
    // Find and return, or return default.
    auto it = mTopicCounts.find(actor + noun + topic);
    if(it != mTopicCounts.end())
    {
        return it->second;
    }
    return 0;
}

void GameProgress::SetTopicCount(const std::string& noun, const std::string& topic, int count)
{
    SetTopicCount(Scene::GetEgoName(), noun, topic, count);
}

void GameProgress::SetTopicCount(const std::string& actor, const std::string& noun, const std::string& topic, int count)
{
    mTopicCounts[actor + noun + topic] = count;
}

void GameProgress::IncTopicCount(const std::string& noun, const std::string& topic)
{
    IncTopicCount(Scene::GetEgoName(), noun, topic);
}

void GameProgress::IncTopicCount(const std::string& actor, const std::string& noun, const std::string& topic)
{
    ++mTopicCounts[actor + noun + topic];
}

int GameProgress::GetNounVerbCount(const std::string& noun, const std::string& verb) const
{
    return GetNounVerbCount(Scene::GetEgoName(), noun, verb);
}

int GameProgress::GetNounVerbCount(const std::string& actor, const std::string& noun, const std::string& verb) const
{
    // Find and return, or return default.
    auto it = mNounVerbCounts.find(actor + noun + verb);
    if(it != mNounVerbCounts.end())
    {
        return it->second;
    }
    return 0;
}

void GameProgress::SetNounVerbCount(const std::string& noun, const std::string& verb, int count)
{
    SetNounVerbCount(Scene::GetEgoName(), noun, verb, count);
}

void GameProgress::SetNounVerbCount(const std::string& actor, const std::string& noun, const std::string& verb, int count)
{
    mNounVerbCounts[actor + noun + verb] = count;
}

void GameProgress::IncNounVerbCount(const std::string& noun, const std::string& verb)
{
    IncNounVerbCount(Scene::GetEgoName(), noun, verb);
}

void GameProgress::IncNounVerbCount(const std::string& actor, const std::string& noun, const std::string& verb)
{
    ++mNounVerbCounts[actor + noun + verb];
}

void GameProgress::OnPersist(PersistState& ps)
{
    ps.Xfer(PERSIST_VAR(mMaxScore));
    ps.Xfer(PERSIST_VAR(mScore));

    ps.Xfer(PERSIST_VAR(mScoreEventFlags));

    mTimeblock.OnPersist(ps);
    mLastTimeblock.OnPersist(ps);

    ps.Xfer(PERSIST_VAR(mChangingTimeblock));

    // Because I'm grabbing the flags from inside the FlagSet, auto-generating the variable name doesn't look right...
    // Maybe should just put OnPersist in FlagSet?
    ps.Xfer("mGameFlags", mGameFlags.GetFlags());

    ps.Xfer(PERSIST_VAR(mChatCounts));
    ps.Xfer(PERSIST_VAR(mTopicCounts));
    ps.Xfer(PERSIST_VAR(mNounVerbCounts));
    ps.Xfer(PERSIST_VAR(mGameVariables));
}
