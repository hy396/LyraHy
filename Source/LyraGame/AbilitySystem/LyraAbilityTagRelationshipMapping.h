// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "LyraAbilityTagRelationshipMapping.generated.h"

class UObject;

/**
 * 单条"技能标签关系"配置。
 * 含义：凡是带有 AbilityTag 的技能，都会对别的技能产生下面这些影响。
 */
USTRUCT()
struct FLyraAbilityTagRelationship
{
	GENERATED_BODY()

	/** 这条关系所针对的标签（单个标签；一个技能可以命中多条关系）。 */
	UPROPERTY(EditAnywhere, Category = Ability, meta = (Categories = "Gameplay.Action"))
	FGameplayTag AbilityTag;

	/** 带此标签的技能激活期间，要【阻塞】哪些标签的技能（让它们无法激活）。 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer AbilityTagsToBlock;

	/** 带此标签的技能激活时，要【取消】掉哪些标签的技能（把正在跑的打断了）。 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer AbilityTagsToCancel;

	/** 带此标签的技能，会被隐式追加这些"激活必需标签"（少了这些标签就激活不了）。 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer ActivationRequiredTags;

	/** 带此标签的技能，会被隐式追加这些"激活阻塞标签"（有这些标签就激活不了）。 */
	UPROPERTY(EditAnywhere, Category = Ability)
	FGameplayTagContainer ActivationBlockedTags;
};


/**
 * ULyraAbilityTagRelationshipMapping —— 技能标签关系映射表（数据资产）
 *
 *	用来集中配置"哪些技能互相阻塞 / 互相取消"，避免在几十个技能蓝图里散落硬编码的标签判断。
 *	典型用法：TagRelationships_ShooterHero 里配置"开火时阻塞换弹"、"冲刺时取消瞄准"之类。
 *	这份表通过 ULyraAbilitySystemComponent::SetTagRelationshipMapping 注入，
 *	之后每次技能激活/结束都会查表推导额外标签。
 */
UCLASS()
class ULyraAbilityTagRelationshipMapping : public UDataAsset
{
	GENERATED_BODY()

private:
	/** 所有标签关系配置条目。 */
	UPROPERTY(EditAnywhere, Category = Ability, meta=(TitleProperty="AbilityTag"))
	TArray<FLyraAbilityTagRelationship> AbilityTagRelationships;

public:
	/** 根据技能标签，查出应当【阻塞】和【取消】的其他标签。 */
	void GetAbilityTagsToBlockAndCancel(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutTagsToBlock, FGameplayTagContainer* OutTagsToCancel) const;

	/** 根据技能标签，查出额外的【激活必需】和【激活阻塞】标签。 */
	void GetRequiredAndBlockedActivationTags(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer* OutActivationRequired, FGameplayTagContainer* OutActivationBlocked) const;

	/** 判断这些技能标签是否会被传入的 ActionTag 取消掉。 */
	bool IsAbilityCancelledByTag(const FGameplayTagContainer& AbilityTags, const FGameplayTag& ActionTag) const;
};
