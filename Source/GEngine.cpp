//
// GEngine.cpp
//
// Clark Kromenaker
//
#include "GEngine.h"

#include <SDL2/SDL.h>

#include "Actor.h"
#include "ButtonIconManager.h"
#include "CharacterManager.h"
#include "Debug.h"
#include "Scene.h"
#include "Services.h"

GEngine* GEngine::inst = nullptr;
std::vector<Actor*> GEngine::mActors;

GEngine::GEngine() : mRunning(false)
{
    inst = this;
}

bool GEngine::Initialize()
{
    // Initialize asset manager.
    Services::SetAssets(&mAssetManager);
    
    // Add "Assets" directory as a possible location for any file.
    mAssetManager.AddSearchPath("Assets/");
    
    // Initialize input.
    Services::SetInput(&mInputManager);
    
    // Initialize renderer.
    if(!mRenderer.Initialize())
    {
        return false;
    }
    Services::SetRenderer(&mRenderer);
    
    // Initialize audio.
    if(!mAudioManager.Initialize())
    {
        return false;
    }
    Services::SetAudio(&mAudioManager);
    
    // For simplicity right now, let's just load all barns at once.
    mAssetManager.LoadBarn("ambient.brn");
    mAssetManager.LoadBarn("common.brn");
    mAssetManager.LoadBarn("core.brn");
    mAssetManager.LoadBarn("day1.brn");
    mAssetManager.LoadBarn("day2.brn");
    mAssetManager.LoadBarn("day3.brn");
    mAssetManager.LoadBarn("day23.brn");
    mAssetManager.LoadBarn("day123.brn");
    
    // Initialize sheep manager.
    Services::SetSheep(&mSheepManager);
    
    //SDL_Log(SDL_GetBasePath());
    //SDL_Log(SDL_GetPrefPath("Test", "GK3"));
	
	// Load cursors and use the default one to start.
	mDefaultCursor = mAssetManager.LoadCursor("C_POINT.CUR");
	mHighlightRedCursor = mAssetManager.LoadCursor("C_ZOOM.CUR");
	mHighlightBlueCursor = mAssetManager.LoadCursor("C_ZOOM_2.CUR");
	mWaitCursor = mAssetManager.LoadCursor("C_WAIT.CUR");
	UseDefaultCursor();
	
	//mAssetManager.LoadVertexAnimation("R25CHAIR_GABR25SIT.ACT");
    //mAssetManager.WriteBarnAssetToFile("R25CHAIR_GABR25SIT.ACT");
    //mAssetManager.WriteAllBarnAssetsToFile(".CUR", "Cursors");
	
	// Load button icon manager.
	Services::Set<ButtonIconManager>(new ButtonIconManager());
	
	// Load character configs.
	Services::Set<CharacterManager>(new CharacterManager());
	
    LoadScene("R25");
    return true;
}

void GEngine::Shutdown()
{
	DeleteAllActors();
    
    mRenderer.Shutdown();
    mAudioManager.Shutdown();
    
    //TODO: Ideally, I don't want the engine to know about SDL.
    SDL_Quit();
}

void GEngine::Run()
{
    // We are running!
    mRunning = true;
    
    // Loop until not running anymore.
    while(mRunning)
    {
		// Our main loop: inputs, updates, outputs.
        ProcessInput();
        Update();
        GenerateOutputs();
		
		// After frame is done, check whether we need a scene change.
		LoadSceneInternal();
    }
}

void GEngine::Quit()
{
    mRunning = false;
}

std::string GEngine::GetCurrentTimeCode()
{
    // Depending on day/hour, returns something like "110A".
	// Add A/P (for am/pm) on end, depending on hour.
    std::string ampm = (mHour <= 11) ? "A" : "P";
	
	// The hour is 24-hour based internally.
	// Subtract 12, if over 12, to get it 12-hour based.
    int hour = mHour > 12 ? mHour - 12 : mHour;
	
	// If hour is single digit, prepend a zero.
	std::string hourStr = std::to_string(hour);
	if(hour < 10)
	{
		hourStr = "0" + hourStr;
	}
	
	// Put it all together.
	return std::to_string(mDay) + hourStr + ampm;
}

void GEngine::UseDefaultCursor()
{
	if(mDefaultCursor != nullptr)
	{
		mActiveCursor = mDefaultCursor;
		mActiveCursor->Activate();
	}
}

void GEngine::UseHighlightCursor()
{
	if(mHighlightRedCursor != nullptr)
	{
		mActiveCursor = mHighlightRedCursor;
		mActiveCursor->Activate();
	}
}

void GEngine::UseWaitCursor()
{
	if(mWaitCursor != nullptr)
	{
		mActiveCursor = mWaitCursor;
		mActiveCursor->Activate();
	}
}

void GEngine::ProcessInput()
{
    // Update the input manager.
    // Retrieve input device states for us to use.
    mInputManager.Update();
    
    // We'll poll for events here. Catch the quit event.
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT:
                Quit();
                break;
        }
    }
    
    // Quit game on escape press for now.
    if(mInputManager.IsKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        Quit();
    }
}

void GEngine::Update()
{
    // Tracks the next "GetTicks" value that is acceptable to perform an update.
    static int nextTicks = 0;
    
    // Tracks the last ticks value each time we run this loop.
    static uint32_t lastTicks = 0;
    
    // Limit to ~60FPS. "nextTicks" is always +16 at start of frame.
    // If we get here again and 16ms have not passed, we wait.
	while(SDL_GetTicks() < nextTicks) { }
	
    // Get current ticks and save next ticks as +16. Limits FPS to ~60.
    uint32_t currentTicks = SDL_GetTicks();
    nextTicks = currentTicks + 16;
	
    // Calculate the time delta.
	uint32_t deltaTicks = currentTicks - lastTicks;
    float deltaTime = deltaTicks * 0.001f;
	
	// Save last ticks for next frame.
    lastTicks = currentTicks;
    
    // Limit the time delta. At least 0s, and at most, 0.05s.
	if(deltaTime < 0.0f) { deltaTime = 0.0f; }
    if(deltaTime > 0.05f) { deltaTime = 0.05f; }
    
    // Update all actors.
    for(int i = 0; i < mActors.size(); i++)
    {
        mActors[i]->Update(deltaTime);
    }
	
	// Delete any dead actors...carefully.
	auto it = mActors.begin();
	while(it != mActors.end())
	{
		if((*it)->GetState() == Actor::State::Dead)
		{
			Actor* actor = (*it);
			it = mActors.erase(it);
			delete actor;
		}
		else
		{
			++it;
		}
	}
    
    // Also update audio system (before or after actors?)
    mAudioManager.Update(deltaTime);
    
    // Update active cursor.
    if(mCursor != nullptr)
    {
        mCursor->Update(deltaTime);
    }
	
	// Update debug visualizations.
	Debug::Update(deltaTime);
}

void GEngine::GenerateOutputs()
{
    mRenderer.Clear();
    mRenderer.Render();
    mRenderer.Present();
}

void GEngine::AddActor(Actor* actor)
{
    mActors.push_back(actor);
}

void GEngine::RemoveActor(Actor* actor)
{
    auto it = std::find(mActors.begin(), mActors.end(), actor);
    if(it != mActors.end())
    {
        mActors.erase(it);
    }
}

void GEngine::LoadSceneInternal()
{
	if(mSceneToLoad.empty()) { return; }
	
	// Delete the current scene, if any.
	if(mScene != nullptr)
	{
		delete mScene;
		mScene = nullptr;
	}
	
	// Get rid of all actors between scenes.
	DeleteAllActors();
	
	// Create the new scene.
	mScene = new Scene(mSceneToLoad, GetCurrentTimeCode());
	
	// Trigger any on-enter actions.
	mScene->OnSceneEnter();
	
	// Clear scene load request.
	mSceneToLoad.clear();
}

void GEngine::DeleteAllActors()
{
	// Delete all actors.
	// Since actor destructor removes from this list, can't iterate and delete.
	while(!mActors.empty())
	{
		delete mActors.back();
	}
}
