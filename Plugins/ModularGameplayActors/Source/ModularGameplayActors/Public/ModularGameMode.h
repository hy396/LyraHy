// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameMode.h"

#include "ModularGameMode.generated.h"

class UObject;

/**
 * AModularGameModeBase —— 精简版（派生自 AGameModeBase）的模块化 GameMode。
 *
 * 应当搭配 AModularGameStateBase 使用。构造函数里已经把配套的
 * GameState / PlayerController / PlayerState / Pawn 类预设成了对应的 Modular 版本，
 * 因此无需在蓝图里手动指定，开箱即用。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AModularGameModeBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};

/**
 * AModularGameMode —— 完整版（派生自 AGameMode，含比赛流程）的模块化 GameMode。
 *
 * 应当搭配 AModularGameState 使用。同 AModularGameModeBase 一样，
 * 构造函数里已预设好整套 Modular 配套类。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AModularGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
};
