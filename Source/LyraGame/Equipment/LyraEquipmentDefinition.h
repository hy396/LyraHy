// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Templates/SubclassOf.h"

#include "LyraEquipmentDefinition.generated.h"

class AActor;
class ULyraAbilitySet;
class ULyraEquipmentInstance;

USTRUCT()
struct FLyraEquipmentActorToSpawn
{
	GENERATED_BODY()

	FLyraEquipmentActorToSpawn()
	{}
	// 要生成的 Actor 类
	UPROPERTY(EditAnywhere, Category=Equipment)
	TSubclassOf<AActor> ActorToSpawn;
	// 要附加到的插槽名称
	UPROPERTY(EditAnywhere, Category=Equipment)
	FName AttachSocket;
	// 附加时使用的相对变换
	UPROPERTY(EditAnywhere, Category=Equipment)
	FTransform AttachTransform;
};


/**
 * ULyraEquipmentDefinition
 *
 * Definition of a piece of equipment that can be applied to a pawn
 * 可应用到 Pawn 的一件装备的定义。
 */
UCLASS(Blueprintable, Const, Abstract, BlueprintType)
class ULyraEquipmentDefinition : public UObject
{
	GENERATED_BODY()

public:
	ULyraEquipmentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 要生成的装备实例类
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TSubclassOf<ULyraEquipmentInstance> InstanceType;

	// 装备时授予的 Gameplay Ability Set 列表
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<TObjectPtr<const ULyraAbilitySet>> AbilitySetsToGrant;

	// 装备时在 Pawn 上生成的 Actor 列表
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TArray<FLyraEquipmentActorToSpawn> ActorsToSpawn;
};
