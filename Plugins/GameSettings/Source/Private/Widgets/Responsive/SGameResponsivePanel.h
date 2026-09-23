// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SCompoundWidget.h"

class FArrangedChildren;
class SWidget;
struct FGeometry;

/**
 * SGameResponsivePanel —— 响应式布局面板的 Slate 实现。
 *
 * 内部基于 SGridPanel 实现：先按可用宽度算出一排放得下几个子控件，
 * 再把它们逐个填进网格的行列里，从而实现"放不下就换行"的效果。
 */
class SGameResponsivePanel : public SCompoundWidget
{
public:

	typedef SGridPanel::FSlot FSlot;

public:

	SLATE_BEGIN_ARGS(SGameResponsivePanel)
	{
		_Visibility = EVisibility::SelfHitTestInvisible;
	}

	SLATE_END_ARGS()

public:

	SGameResponsivePanel();

	/**
	 * Construct this widget
	 *
	 * @param	InArgs	The declaration data for this widget
	 */
	void Construct(const FArguments& InArgs);

	/**
	 * Adds a content slot.
	 *
	 * @return The added slot.
	 */
	FSlot& AddSlot();

	/**
	 * Removes a particular content slot.
	 *
	 * @param SlotWidget The widget in the slot to remove.
	 */
	int32 RemoveSlot(const TSharedRef<SWidget>& SlotWidget);

	/**
	 * Removes all slots from the panel.
	 */
	void ClearChildren();

	/** 开关"放不下就自动换行"。 */
	void EnableVerticalStacking(const bool bCanVerticallyWrap);

protected:
	// Begin SWidget overrides.
	virtual bool CustomPrepass(float LayoutScaleMultiplier) override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual float GetRelativeLayoutScale(int32 ChildIndex, float LayoutScaleMultiplier) const override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const;
	// End SWidget overrides.

	bool ShouldWrap() const;

	void RefreshResponsiveness();
	void RefreshLayout();

protected:

	TSharedRef<SGridPanel> InnerGrid;
	TArray<SGridPanel::FSlot*> InnerSlots;

	FVector2D PhysialScreenSize = FVector2D(0, 0);
	float Scale = 1;

	uint8 bCanWrapVertically : 1;
};
