// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * Pawn 数据资产（不可变）：用一个资产描述「这个 Pawn 是什么样」。
 * 包含：要生成的 Pawn 类、要授予的能力集、标签关系映射、输入配置、默认摄像机模式。
 * 换 PawnData 就等于换一套角色玩法，这也是 Lyra 用 Experience 切换玩法的关键抓手。
 */
#include "Engine/DataAsset.h"

#include "LyraPawnData.generated.h"

class APawn;
class ULyraAbilitySet;
class ULyraAbilityTagRelationshipMapping;
class ULyraCameraMode;
class ULyraInputConfig;
class UObject;


// 定义一个 Pawn 所需的全部数据，运行期只读
/**
 * ULyraPawnData
 *
 *	Non-mutable data asset that contains properties used to define a pawn.
 */
UCLASS(BlueprintType, Const, Meta = (DisplayName = "Lyra Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class LYRAGAME_API ULyraPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	ULyraPawnData(const FObjectInitializer& ObjectInitializer);

public:

	// 要实例化的 Pawn 类（通常继承 ALyraPawn 或 ALyraCharacter）
	// Class to instantiate for this pawn (should usually derive from ALyraPawn or ALyraCharacter).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Pawn")
	TSubclassOf<APawn> PawnClass;

	// 要授予该 Pawn 的能力集列表
	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Abilities")
	TArray<TObjectPtr<ULyraAbilitySet>> AbilitySets;

	// 该 Pawn 使用的技能标签关系映射（互斥/取消/必须等规则）
	// What mapping of ability tags to use for actions taking by this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Abilities")
	TObjectPtr<ULyraAbilityTagRelationshipMapping> TagRelationshipMapping;

	// 玩家控制该 Pawn 时使用的输入配置
	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Input")
	TObjectPtr<ULyraInputConfig> InputConfig;

	// 玩家控制该 Pawn 时使用的默认摄像机模式
	// Default camera mode used by player controlled pawns.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Camera")
	TSubclassOf<ULyraCameraMode> DefaultCameraMode;
};
