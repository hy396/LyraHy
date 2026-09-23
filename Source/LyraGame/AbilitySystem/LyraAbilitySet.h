// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActiveGameplayEffectHandle.h"
#include "Engine/DataAsset.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"

#include "GameplayAbilitySpecHandle.h"
#include "LyraAbilitySet.generated.h"

class UAttributeSet;
class UGameplayEffect;
class ULyraAbilitySystemComponent;
class ULyraGameplayAbility;
class UObject;


/**
 * FLyraAbilitySet_GameplayAbility
 *
 *	技能集里"授予一个技能"所需的配置。
 */
USTRUCT(BlueprintType)
struct FLyraAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:

	/** 要授予的技能类。 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULyraGameplayAbility> Ability = nullptr;

	/** 授予时的技能等级。 */
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	/** 触发这个技能的输入标签（如 InputTag.Ability.Dash）。留空表示不由输入触发。 */
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};


/**
 * FLyraAbilitySet_GameplayEffect
 *
 *	技能集里"授予一个 GameplayEffect"所需的配置（常用来挂初始属性加成、被动效果）。
 */
USTRUCT(BlueprintType)
struct FLyraAbilitySet_GameplayEffect
{
	GENERATED_BODY()

public:

	/** 要授予的 GameplayEffect 类。 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	/** 授予时的效果等级。 */
	UPROPERTY(EditDefaultsOnly)
	float EffectLevel = 1.0f;
};

/**
 * FLyraAbilitySet_AttributeSet
 *
 *	技能集里"授予一个属性集"所需的配置（例如 LyraHealthSet、LyraCombatSet）。
 */
USTRUCT(BlueprintType)
struct FLyraAbilitySet_AttributeSet
{
	GENERATED_BODY()

public:
	/** 要授予的属性集类。 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAttributeSet> AttributeSet;

};

/**
 * FLyraAbilitySet_GrantedHandles
 *
 *	记录"本次授予出去的东西"的句柄集合。
 *	授予时把句柄存进来，之后调用 TakeFromAbilitySystem 就能精确地把这一批东西全部收回，
 *	不会误伤角色身上其他来源的技能/效果。
 */
USTRUCT(BlueprintType)
struct FLyraAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:

	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	void AddAttributeSet(UAttributeSet* Set);

	/** 把之前记录的所有技能 / 效果 / 属性集从指定 ASC 上移除（撤销本次授予）。 */
	void TakeFromAbilitySystem(ULyraAbilitySystemComponent* LyraASC);

protected:

	/** 已授予技能的句柄列表。 */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	/** 已激活 GameplayEffect 的句柄列表。 */
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;

	/** 已授予的属性集指针列表。 */
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> GrantedAttributeSets;
};


/**
 * ULyraAbilitySet
 *
 *	"技能集"数据资产（只读，不保存运行时状态）。
 *
 *	把一个角色该有的技能、GameplayEffect、属性集打包成一份配置，
 *	由 GameFeatureAction_AddAbilities 或 PawnData 在合适的时机一次性授予给 ASC。
 *	例如 AbilitySet_ShooterHero 就打包了跳跃、冲刺、手雷、近战等一整套英雄技能。
 */
UCLASS(BlueprintType, Const)
class ULyraAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	ULyraAbilitySet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * 把本技能集授予给指定的 ASC。
	 * @param OutGrantedHandles 传出本次授予产生的所有句柄，之后可用它精确撤销这一批授予。
	 * @param SourceObject      授予来源对象，便于追溯是谁给的（可为 nullptr）。
	 */
	void GiveToAbilitySystem(ULyraAbilitySystemComponent* LyraASC, FLyraAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

protected:

	/** 授予时要给的技能列表。 */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities", meta=(TitleProperty=Ability))
	TArray<FLyraAbilitySet_GameplayAbility> GrantedGameplayAbilities;

	/** 授予时要给的 GameplayEffect 列表。 */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects", meta=(TitleProperty=GameplayEffect))
	TArray<FLyraAbilitySet_GameplayEffect> GrantedGameplayEffects;

	/** 授予时要挂的属性集列表。 */
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Sets", meta=(TitleProperty=AttributeSet))
	TArray<FLyraAbilitySet_AttributeSet> GrantedAttributes;
};
