// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"

#include "ModularPlayerController.generated.h"

class UObject;

/**
 * AModularPlayerController —— 可被 GameFeature 插件动态扩展的玩家控制器基类。
 *
 * 除了标准的“登记 / 派发就绪 / 注销”三件事之外，本类还额外做了转发：
 *   - ReceivedPlayer() 转发给身上所有 UControllerComponent
 *   - PlayerTick()    每帧转发给身上所有 UControllerComponent
 * 注意它与别的 Modular* 类不同：就绪事件不是在 BeginPlay 派发，而是在 ReceivedPlayer 派发，
 * 因为 PlayerController 必须等到真正分配了 Player 之后才可用。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface

	//~ Begin APlayerController interface
	virtual void ReceivedPlayer() override;
	virtual void PlayerTick(float DeltaTime) override;
	//~ End APlayerController interface
};
