//
// ActionBar.cpp
//
// Clark Kromenaker
//
#include "ActionBar.h"

#include "InventoryManager.h"
#include "Scene.h"
#include "StringUtil.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UILabel.h"
#include "VerbManager.h"

ActionBar::ActionBar() : Actor(TransformType::RectTransform)
{
	// Create canvas, to contain the UI components.
	mCanvas = AddComponent<UICanvas>();
	
	// Since we will set the action bar's position based on mouse position, set the anchor to the lower-left corner.
	RectTransform* rectTransform = GetComponent<RectTransform>();
	rectTransform->SetSizeDelta(0.0f, 0.0f);
	rectTransform->SetAnchorMin(Vector2::Zero);
	rectTransform->SetAnchorMax(Vector2::One);
	
	// Create button holder - it holds the buttons and we move it around the screen.
	Actor* buttonHolderActor = new Actor(Actor::TransformType::RectTransform);
	mButtonHolder = buttonHolderActor->GetComponent<RectTransform>();
	mButtonHolder->SetParent(rectTransform);
	mButtonHolder->SetAnchorMin(Vector2::Zero);
	mButtonHolder->SetAnchorMax(Vector2::Zero);
	mButtonHolder->SetPivot(0.5f, 0.5f);
	
	// To have button holder appear in correct spot, we need the holder to be the right height.
	// So, just use one of the buttons to get a valid height.
	VerbIcon& cancelVerbIcon = Services::Get<VerbManager>()->GetVerbIcon("CANCEL");
	mButtonHolder->SetSizeDelta(cancelVerbIcon.GetWidth(), cancelVerbIcon.GetWidth());
	
	// Hide by default.
	Hide();
}

void ActionBar::Show(const std::string& noun, VerbType verbType, std::vector<const Action*> actions, std::function<void(const Action*)> executeCallback, std::function<void()> cancelCallback)
{
	// Hide if not already hidden (make sure buttons are freed).
	mHideCallback = nullptr;
	Hide();
	
	// If we don't have any actions, don't need to do anything!
	if(actions.size() <= 0) { return; }
	
	// Save hide callback.
	mHideCallback = cancelCallback;
	
	// Iterate over all desired actions and show a button for it.
	bool inventoryShowing = Services::Get<InventoryManager>()->IsInventoryShowing();
	VerbManager* verbManager = Services::Get<VerbManager>();
	int buttonIndex = 0;
	for(int i = 0; i < actions.size(); ++i)
	{
		// Kind of a HACK, but it does the trick...if inventory is showing, hide all PICKUP verbs.
		// You can't PICKUP something you already have. And the PICKUP icon is used in inventory to represent "make active".
		if(inventoryShowing && StringUtil::EqualsIgnoreCase(actions[i]->verb, "PICKUP"))
		{
			continue;
		}
		
		// Add button with appropriate icon.
		UIButton* actionButton = nullptr;
		if(verbType == VerbType::Normal)
		{
			VerbIcon& buttonIcon = verbManager->GetVerbIcon(actions[i]->verb);
			actionButton = AddButton(buttonIndex, buttonIcon);
		}
		else if(verbType == VerbType::Topic)
		{
			VerbIcon& buttonIcon = verbManager->GetTopicIcon(actions[i]->verb);
			actionButton = AddButton(buttonIndex, buttonIcon);
		}
		if(actionButton == nullptr) { continue; }
		++buttonIndex;
		
		// Set up button callback to execute the action.
		const Action* action = actions[i];
		actionButton->SetPressCallback([this, action, executeCallback]() {
			// Hide action bar on button press.
			this->Hide();
			
			// If callback was provided for press, pass handling the press off to that.
			// If no callback was provided, simply play the action!
			if(executeCallback != nullptr)
			{
				executeCallback(action);
			}
			else
			{
				Services::Get<ActionManager>()->ExecuteAction(action);
			}
		});
	}
	
	// Show inventory item button IF this is a normal action bar (not topic chooser).
	if(verbType == VerbType::Normal)
	{
		// Get active inventory item for current ego.
		const std::string& egoName = GEngine::Instance()->GetScene()->GetEgoName();
		std::string activeItemName = Services::Get<InventoryManager>()->GetActiveInventoryItem(egoName);
		
		// Show inventory button if there's an active inventory item AND it is not the object we're interacting with.
		// In other words, don't allow using an object on itself!
		mHasInventoryItemButton = !activeItemName.empty() && !StringUtil::EqualsIgnoreCase(activeItemName, noun);
		if(mHasInventoryItemButton)
		{
			VerbIcon& invVerbIcon = verbManager->GetInventoryIcon(activeItemName);
			UIButton* invButton = AddButton(buttonIndex, invVerbIcon);
			++buttonIndex;
			
			// Create callback for inventory button press.
			const Action* invAction = Services::Get<ActionManager>()->GetAction(actions[0]->noun, activeItemName);
			invButton->SetPressCallback([this, invAction, executeCallback]() {
				// Hide action bar on button press.
				this->Hide();
				
				// Execute the action, which will likely run some SheepScript.
				executeCallback(invAction);
			});
		}
	}
	
	// Always put cancel button on the end.
	VerbIcon& cancelVerbIcon = verbManager->GetVerbIcon("CANCEL");
	UIButton* cancelButton = AddButton(buttonIndex, cancelVerbIcon);
	
	// Just hide the bar when cancel is pressed.
	cancelButton->SetPressCallback(std::bind(&ActionBar::Hide, this));
	
	// Refresh layout after adding all buttons to position everything correctly.
	RefreshButtonLayout();
	
	// Position action bar at pointer.
	// This also makes sure it doesn't go offscreen.
	CenterOnPointer();
	
	// It's showing now!
	mButtonHolder->GetOwner()->SetActive(true);
}

void ActionBar::Hide()
{
	// Disable all buttons and put them on the free stack.
	for(auto& button : mButtons)
	{
		button->SetEnabled(false);
		mFreeButtons.push(button);
	}
	mButtons.clear();
	
	// Button holder inactive = no children show.
	mButtonHolder->GetOwner()->SetActive(false);
	
	// Call hide callback.
	if(mHideCallback != nullptr)
	{
		mHideCallback();
		mHideCallback = nullptr;
	}
}

bool ActionBar::IsShowing() const
{
	return mButtonHolder->IsActiveAndEnabled();
}

void ActionBar::AddVerbToFront(const std::string& verb, std::function<void()> callback)
{
	// Add button at index 0.
	VerbIcon& icon = Services::Get<VerbManager>()->GetVerbIcon(verb);
	UIButton* button = AddButton(0, icon);
	button->SetPressCallback([this, callback]() {
		this->Hide();
		callback();
	});
	
	// Refresh button positions and move bar to pointer.
	RefreshButtonLayout();
	CenterOnPointer();
}

void ActionBar::AddVerbToBack(const std::string& verb, std::function<void()> callback)
{
	VerbIcon& icon = Services::Get<VerbManager>()->GetVerbIcon(verb);
	
	// Action bar order is always [VERBS][INV_ITEM][CANCEL]
	// So, skip 1 for cancel button, and maybe skip another one if inventory item is shown.
	int skipCount = 1;
	if(mHasInventoryItemButton)
	{
		skipCount = 2;
	}
	
	// Add button with callback at index.
	UIButton* button = AddButton(static_cast<int>(mButtons.size() - skipCount), icon);
	button->SetPressCallback([this, callback]() {
		this->Hide();
		callback();
	});
	
	// Refresh button positions and move bar to pointer.
	RefreshButtonLayout();
	CenterOnPointer();
}

void ActionBar::OnUpdate(float deltaTime)
{
	if(IsShowing() && Services::GetInput()->IsKeyDown(SDL_SCANCODE_BACKSPACE))
	{
		Hide();
	}
}

UIButton* ActionBar::AddButton(int index, const VerbIcon& buttonIcon)
{
	// Reuse a free button or create a new one.
	UIButton* button = nullptr;
	if(mFreeButtons.size() > 0)
	{
		button = mFreeButtons.top();
		mFreeButtons.pop();
		
		// Make sure button is enabled.
		button->SetEnabled(true);
		
		// Make sure any old callbacks are no longer set (since we recycle the buttons).
		button->SetPressCallback(nullptr);
	}
	else
	{
		Actor* buttonActor = new Actor(Actor::TransformType::RectTransform);
		buttonActor->GetTransform()->SetParent(mButtonHolder);
		
		button = buttonActor->AddComponent<UIButton>();
		
		// Add button as a widget.
		mCanvas->AddWidget(button);
	}
	
	// Put into buttons array at desired position.
	mButtons.insert(mButtons.begin() + index, button);
	
	// Set size correctly.
	button->GetRectTransform()->SetSizeDelta(buttonIcon.GetWidth(), buttonIcon.GetWidth());
	
	// Show correct icon on button.
	button->SetUpTexture(buttonIcon.upTexture);
	button->SetDownTexture(buttonIcon.downTexture);
	button->SetHoverTexture(buttonIcon.hoverTexture);
	button->SetDisabledTexture(buttonIcon.disableTexture);
	return button;
}

void ActionBar::RefreshButtonLayout()
{
	// Iterate over all buttons, positioning each one right after the previous one.
	float xPos = 0.0f;
	for(auto& button : mButtons)
	{		
		// Position correctly, relative to previous buttons.
		RectTransform* buttonRT = button->GetRectTransform();
		buttonRT->SetAnchor(Vector2::Zero);
		buttonRT->SetPivot(0.0f, 0.0f);
		buttonRT->SetAnchoredPosition(Vector2(xPos, 0.0f));
		
		xPos += buttonRT->GetSize().x;
	}
	
	// Update the button holder to match the size of all buttons.
	mButtonHolder->SetSizeDeltaX(xPos);
}

void ActionBar::CenterOnPointer()
{
	// Position action bar at mouse position.
	mButtonHolder->SetAnchoredPosition(Services::GetInput()->GetMousePosition());
	
	// Keep inside the screen.
	//TODO: Seems like this might be generally useful...perhaps a RectTransform "KeepInRect(Rect)" function?
	// Get min/max for rect of the holder.
	Rect screenRect = mCanvas->GetRectTransform()->GetWorldRect();
	//Vector2 screenRectMin = screenRect.GetMin();
	Vector2 screenRectMax = screenRect.GetMax();
	
	Rect buttonHolderRect = mButtonHolder->GetWorldRect();
	Vector2 min = buttonHolderRect.GetMin();
	Vector2 max = buttonHolderRect.GetMax();
	
	Vector2 anchoredPos = mButtonHolder->GetAnchoredPosition();
	if(min.x < 0)
	{
		anchoredPos.x = anchoredPos.x - min.x;
	}
	if(max.x > screenRectMax.x)
	{
		anchoredPos.x = anchoredPos.x - (max.x - screenRectMax.x);
	}
	if(min.y < 0)
	{
		anchoredPos.y = anchoredPos.y - min.y;
	}
	if(max.y > screenRectMax.y)
	{
		anchoredPos.y = anchoredPos.y - (max.y - screenRectMax.y);
	}
	mButtonHolder->SetAnchoredPosition(anchoredPos);
}
