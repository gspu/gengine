//
// Clark Kromenaker
//
// A collection of UI components that are logically related and
// updated/drawn/managed together.
//
// A UI "canvas" could represent one distinct UI screen or dialog.
// E.g. a Main Menu, an Options Screen, a Popup, a Tooltip.
//
#pragma once
#include "UIWidget.h"

#include <climits>

class UICanvas : public UIWidget
{
	TYPE_DECL();
public:
	static const std::vector<UICanvas*>& GetCanvases() { return sCanvases; }
	static void UpdateInput();
	static bool DidWidgetEatInput() { return sMouseOverWidget != nullptr; }
    static void NotifyWidgetDestruct(UIWidget* widget);

	UICanvas(Actor* owner);
    UICanvas(Actor* owner, int order);
	~UICanvas();
	
	void Render() override;
	
	void AddWidget(UIWidget* widget);
	void RemoveWidget(UIWidget* widget);
	void RemoveAllWidgets() { mWidgets.clear(); }

    void SetMasked(bool masked) { mMasked = masked; }
	
private:
	// An array of all canvases that currently exist.
	static std::vector<UICanvas*> sCanvases;
	
	// At any time, the mouse can be over exactly one widget.
	// (at least, unless we add multi-pointer support...shudders)
	static UIWidget* sMouseOverWidget;

    // Desired draw order for the canvas. Zero is drawn before one, one is drawn before two, etc.
    int mDrawOrder = INT_MAX;
	
	// All widgets on this canvas.
    std::vector<UIWidget*> mWidgets;

    bool mMasked = false;
};
