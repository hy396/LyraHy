// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModularPlayerController.h"

#include "CommonPlayerController.generated.h"

class APawn;
class UObject;

/**
 * ACommonPlayerController —— 增强版玩家控制器。
 *
 * 它派生自 AModularPlayerController（因此天然支持 GameFeature 动态挂组件）。
 * 额外做的事情只有一个主题：在 PlayerController / PlayerState / Pawn 各自就位时，
 * 通知所属的 UCommonLocalPlayer，触发那三个 CallAndRegister_ 委托。
 */
UCLASS(config=Game)
class COMMONGAME_API ACommonPlayerController : public AModularPlayerController
{
	GENERATED_BODY()

public:
	ACommonPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ReceivedPlayer() override;	
	virtual void SetPawn(APawn* InPawn) override;
	virtual void OnPossess(class APawn* APawn) override;
	virtual void OnUnPossess() override;
	
protected:
	virtual void OnRep_PlayerState() override;
};
