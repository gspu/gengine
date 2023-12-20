//
// Clark Kromenaker
//
// Base class for any UI element, called a "widget".
// This class is abstract.
//
#pragma once
#include "Component.h"

#include "RectTransform.h"

class UIWidget : public Component
{
    TYPE_DECL();
public:
    UIWidget(Actor* actor);
    virtual ~UIWidget();
    
	virtual void Render() = 0;

    // Called when pointer enters/exits the bounds of this widget.
	virtual void OnPointerEnter() { }
	virtual void OnPointerExit() { }

    // Called when pointer down/up when within the bounds of this widget.
	virtual void OnPointerDown() { }
	virtual void OnPointerUp() { }
	
	RectTransform* GetRectTransform() const { return mRectTransform; }
    
    void SetReceivesInput(bool receivesInput) { mReceivesInput = receivesInput; }
    bool ReceivesInput() const { return mReceivesInput; }
	
protected:
	Matrix4 GetWorldTransformWithSizeForRendering();
    
private:
    // UI widgets usually rely heavily on a RectTransform.
    // So, we cache it here.
    RectTransform* mRectTransform = nullptr;
    
    // If a widget receives input, its "OnPointer" functions are called
    // AND it can block interaction with the scene (since it can "eat" inputs).
    bool mReceivesInput = false;
};
