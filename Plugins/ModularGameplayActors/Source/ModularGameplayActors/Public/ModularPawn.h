// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Pawn.h"

#include "ModularPawn.generated.h"

class UObject;

/**
 * AModularPawn —— 可被 GameFeature 插件动态扩展的 Pawn 基类。
 *
 * 本类自身几乎不含游戏逻辑，只负责在生命周期的合适时机与 UGameFrameworkComponentManager 对接：
 *   登记为组件接收者 → 派发“Actor 就绪”事件 → 销毁时注销。
 */
UCLASS(Blueprintable)
class MODULARGAMEPLAYACTORS_API AModularPawn : public APawn
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface

};
