// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameState.h"

#include "ModularGameState.generated.h"

class UObject;

/**
 * AModularGameStateBase —— 精简版（派生自 AGameStateBase）的模块化 GameState。
 * 应当搭配 AModularGameModeBase 使用。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface
};


/**
 * AModularGameState —— 完整版（派生自 AGameState）的模块化 GameState。
 *
 * 应当搭配 AModularGameMode 使用（此处原版英文注释写的是 “Pair this with a ModularGameState”，
 * 应为笔误，实际配对对象是 GameMode）。
 * 额外重写了 HandleMatchHasStarted：比赛开始时把事件转发给身上所有 UGameStateComponent。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularGameState : public AGameState
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface

protected:
	//~ Begin AGameState interface
	virtual void HandleMatchHasStarted() override;
	//~ Begin AGameState interface
};
