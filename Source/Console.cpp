//
// Console.cpp
//
// Clark Kromenaker
//
#include "Console.h"

#include "Mover.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"

Console::Console(bool mini) : Actor(TransformType::RectTransform)
{
	// Add canvas for UI rendering.
	mCanvas = AddComponent<UICanvas>();
	
	// Create screen-sized canvas.
	RectTransform* canvasTransform = GetComponent<RectTransform>();
	canvasTransform->SetSizeDelta(0.0f, 0.0f);
	canvasTransform->SetAnchorMin(Vector2::Zero);
	canvasTransform->SetAnchorMax(Vector2::One);
	
	// Create background image actor and save transform (so we can move it around).
	Actor* background = new Actor(TransformType::RectTransform);
	mBackgroundTransform = background->GetComponent<RectTransform>();
	mBackgroundTransform->SetParent(canvasTransform);
	
	// Add background image that is tiled.
	// Only difference between mini and full is a different background image.
	UIImage* backgroundImage = background->AddComponent<UIImage>();
	backgroundImage->SetRenderMode(UIImage::RenderMode::Tiled);
	if(mini)
	{
		backgroundImage->SetTexture(Services::GetAssets()->LoadTexture("MINISNAKY"));
	}
	else
	{
		backgroundImage->SetTexture(Services::GetAssets()->LoadTexture("SNAKY"));
	}
	mCanvas->AddWidget(backgroundImage);
	
	// Mini and full consoles have different anchoring properties.
	// Mini is a box that is anchored to a corner of the screen.
	// Full is a panel that fully covers the top or bottom parts of the screen.
	if(mini)
	{
		
	}
	else
	{
		// Anchored along top edge of parent rect (anchor min/max).
		// Grow from top down (pivot).
		// No padding on left/right (sizeDelta.x).
		// Default height of 200 (sizeDelta.y).
		mBackgroundTransform->SetAnchorMin(Vector2(0.0f, 1.0f));
		mBackgroundTransform->SetAnchorMax(Vector2(1.0f, 1.0f));
		mBackgroundTransform->SetPivot(0.5f, 1.0f);
		mBackgroundTransform->SetSizeDelta(0.0f, 200.0f);
		
		// Add horizontal rule for full console.
		Actor* hrActor = new Actor(TransformType::RectTransform);
		RectTransform* hrTransform = hrActor->GetComponent<RectTransform>();
		hrTransform->SetParent(mBackgroundTransform);
		
		// Horizontal rule uses a tiling line image.
		UIImage* hrImage = hrActor->AddComponent<UIImage>();
		hrImage->SetRenderMode(UIImage::RenderMode::Tiled);
		hrImage->SetTexture(Services::GetAssets()->LoadTexture("HORIZONTALRULE"));
		hrImage->SetSizeToTextureSize();
		mCanvas->AddWidget(hrImage);
		
		// Anchor along bottom edge of the console, with enough space for the input line below.
		hrTransform->SetAnchorMin(Vector2(0.0f, 0.0f));
		hrTransform->SetAnchorMax(Vector2(1.0f, 0.0f));
		hrTransform->SetSizeDelta(0.0f, hrTransform->GetSize().GetY());
		hrTransform->SetAnchoredPosition(0.0f, 25.0f);
		
		// Create scrollback actor.
		Actor* scrollbackActor = new Actor(TransformType::RectTransform);
		RectTransform* scrollbackRT = scrollbackActor->GetComponent<RectTransform>();
		scrollbackRT->SetParent(mBackgroundTransform);
		
		// Scrollback takes up big chunk of space above horizontal rule.
		scrollbackRT->SetAnchorMin(Vector2(0.0f, 0.2f));
		scrollbackRT->SetAnchorMax(Vector2(1.0f, 1.0f));
		scrollbackRT->SetSizeDelta(-2.0f, -2.0f);
		scrollbackRT->SetPivot(0.5f, 0.0f);
		
		UILabel* scrollbackText = scrollbackActor->AddComponent<UILabel>();
		mCanvas->AddWidget(scrollbackText);
		
		scrollbackText->SetFont(Services::GetAssets()->LoadFont("F_CONSOLE_DISPLAY"));
		scrollbackText->SetText("GEngine");
	}
	
	// Disable canvas so console is hidden by default.
	mCanvas->SetEnabled(false);
}

void Console::UpdateInternal(float deltaTime)
{
	// Show an indicator when the console key is pressed down, but not yet released.
	if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_GRAVE))
	{
		//TODO: Show "Cain" image.
	}
	
	// On release, toggle the display of the console.
	if(Services::GetInput()->IsKeyUp(SDL_SCANCODE_GRAVE))
	{
		mCanvas->SetEnabled(!mCanvas->GetEnabled());
	}
	
	//TODO: Alt+Up or Alt+Down should increase/decrease the size of the full console.
	
	//TODO: Some key combo also swaps full console from top to bottom.
	
	//TODO: Similarly, some key combo swaps mini console between the four corners of the screen
	
	//TODO: Some key combo allows scrolling through the console buffer.
	
	/*
	Vector2 pivot = mBackgroundTransform->GetPivot();
	if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_LEFT))
	{
		pivot.SetX(pivot.GetX() - 0.1f);
	}
	else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_RIGHT))
	{
		pivot.SetX(pivot.GetX() + 0.1f);
	}
	if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_DOWN))
	{
		pivot.SetY(pivot.GetY() - 0.1f);
	}
	else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_UP))
	{
		pivot.SetY(pivot.GetY() + 0.1f);
	}
	mBackgroundTransform->SetPivot(pivot);
	*/
}
