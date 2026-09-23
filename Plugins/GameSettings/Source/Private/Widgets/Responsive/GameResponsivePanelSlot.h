// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/PanelSlot.h"
#include "SGameResponsivePanel.h"

#include "GameResponsivePanelSlot.generated.h"

class UObject;

/**
 * UGameResponsivePanelSlot —— UGameResponsivePanel 的槽位。
 *
 * 它是 UMG 层对底层 SGameResponsivePanel::FSlot 的包装，
 * 负责把 UMG 侧的属性同步到 Slate 槽位上。
 */
UCLASS()
class UGameResponsivePanelSlot : public UPanelSlot
{
	GENERATED_UCLASS_BODY()

public:
	

public:

	void BuildSlot(TSharedRef<SGameResponsivePanel> GameResponsivePanel);

	// UPanelSlot interface
	virtual void SynchronizeProperties() override;
	// End of UPanelSlot interface

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
	/** 指向底层 Slate 面板中的真实槽位。 */
	SGameResponsivePanel::FSlot* Slot;
};
