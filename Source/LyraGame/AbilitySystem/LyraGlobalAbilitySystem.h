// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayAbilitySpecHandle.h"
#include "Templates/SubclassOf.h"

#include "LyraGlobalAbilitySystem.generated.h"

class UGameplayAbility;
class UGameplayEffect;
class ULyraAbilitySystemComponent;
class UObject;
struct FActiveGameplayEffectHandle;
struct FFrame;
struct FGameplayAbilitySpecHandle;

/** 记录"某个全局技能"被下发到了哪些 ASC 上，以及各自产生的句柄，便于之后逐个收回。 */
USTRUCT()
struct FGlobalAppliedAbilityList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<ULyraAbilitySystemComponent>, FGameplayAbilitySpecHandle> Handles;

	void AddToASC(TSubclassOf<UGameplayAbility> Ability, ULyraAbilitySystemComponent* ASC);
	void RemoveFromASC(ULyraAbilitySystemComponent* ASC);
	void RemoveFromAll();
};

/** 记录"某个全局 GameplayEffect"被下发到了哪些 ASC 上，以及各自的激活句柄。 */
USTRUCT()
struct FGlobalAppliedEffectList
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<TObjectPtr<ULyraAbilitySystemComponent>, FActiveGameplayEffectHandle> Handles;

	void AddToASC(TSubclassOf<UGameplayEffect> Effect, ULyraAbilitySystemComponent* ASC);
	void RemoveFromASC(ULyraAbilitySystemComponent* ASC);
	void RemoveFromAll();
};

/**
 * ULyraGlobalAbilitySystem —— 全局技能系统
 *
 *	用来把某个技能 / GameplayEffect 一次性施加给【本世界内所有】角色，
 *	并且对之后新加入的角色也会自动补发。
 *
 *	典型场景：比赛进入某个阶段时，给全场玩家挂一个"全体加速"或"全体无敌"的 GE；
 *	中途加入的玩家也能自动获得，因为 RegisterASC 会把当前所有全局效果补给它。
 */
UCLASS()
class ULyraGlobalAbilitySystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	ULyraGlobalAbilitySystem();

	/** 给全场所有角色（以及之后新注册的角色）授予指定技能。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Lyra")
	void ApplyAbilityToAll(TSubclassOf<UGameplayAbility> Ability);

	/** 给全场所有角色（以及之后新注册的角色）施加指定 GameplayEffect。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Lyra")
	void ApplyEffectToAll(TSubclassOf<UGameplayEffect> Effect);

	/** 从全场所有角色身上收回指定技能。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Lyra")
	void RemoveAbilityFromAll(TSubclassOf<UGameplayAbility> Ability);

	/** 从全场所有角色身上移除指定 GameplayEffect。 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Lyra")
	void RemoveEffectFromAll(TSubclassOf<UGameplayEffect> Effect);

	/** 把一个 ASC 登记进全局系统，并立即把当前生效的全局技能/效果补发给它。 */
	void RegisterASC(ULyraAbilitySystemComponent* ASC);

	/** 把一个 ASC 从全局系统注销，同时移除它身上所有全局技能/效果。 */
	void UnregisterASC(ULyraAbilitySystemComponent* ASC);

private:
	/** 已下发的全局技能：技能类 → (各 ASC 上的句柄)。 */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayAbility>, FGlobalAppliedAbilityList> AppliedAbilities;

	/** 已下发的全局效果：效果类 → (各 ASC 上的句柄)。 */
	UPROPERTY()
	TMap<TSubclassOf<UGameplayEffect>, FGlobalAppliedEffectList> AppliedEffects;

	/** 当前登记在全局系统里的所有 ASC。 */
	UPROPERTY()
	TArray<TObjectPtr<ULyraAbilitySystemComponent>> RegisteredASCs;
};
