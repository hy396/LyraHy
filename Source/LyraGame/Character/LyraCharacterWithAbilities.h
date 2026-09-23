// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 自带 ASC 的角色类：普通 ALyraCharacter 的 ASC 来自 PlayerState，
// 而这个类自己持有一套 ASC 与属性集，适合不需要 PlayerState 的场景（例如 AI、临时 NPC）。
#include "Character/LyraCharacter.h"

#include "LyraCharacterWithAbilities.generated.h"

class UAbilitySystemComponent;
class ULyraAbilitySystemComponent;
class UObject;

// 普通角色从 PlayerState 上取 ASC，这个类自带一套
// ALyraCharacter typically gets the ability system component from the possessing player state
// This represents a character with a self-contained ability system component.
// 自带 ASC 的角色
UCLASS(Blueprintable)
class LYRAGAME_API ALyraCharacterWithAbilities : public ALyraCharacter
{
	GENERATED_BODY()

public:
	ALyraCharacterWithAbilities(const FObjectInitializer& ObjectInitializer);

	// 组件初始化后：把自己注册成自带 ASC 的 Avatar
	virtual void PostInitializeComponents() override;

	// 返回自带的 ASC
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:

	// 自带的 ASC 子组件
	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "Lyra|PlayerState")
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;
	
	// 自带的生命值属性集
	// Health attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class ULyraHealthSet> HealthSet;
	// 自带的战斗属性集
	// Combat attribute set used by this actor.
	UPROPERTY()
	TObjectPtr<const class ULyraCombatSet> CombatSet;
};
