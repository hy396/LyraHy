// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 体验（Experience）定义资产。
 *
 * 「体验」是 Lyra 的核心概念：一张地图 + 一套玩法规则的组合。
 * 它本身只是一个数据资产，负责声明：要激活哪些 GameFeature 插件、
 * 玩家默认用什么 Pawn、加载/激活/反激活/卸载时要执行哪些 UGameFeatureAction。
 */
#include "Engine/DataAsset.h"
#include "LyraExperienceDefinition.generated.h"

class UGameFeatureAction;
class ULyraPawnData;
class ULyraExperienceActionSet;

// 一次「体验」的完整定义；注意它是 Const 资产，运行时只读
/**
 * Definition of an experience
 */
UCLASS(BlueprintType, Const)
class ULyraExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	ULyraExperienceDefinition();

	// UObject 接口：编辑器下做数据校验
	//~UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	//~End of UObject interface

	// UPrimaryDataAsset 接口：编辑器下把引用的资源登记进 AssetBundle，供异步加载使用
	//~UPrimaryDataAsset interface
#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif
	//~End of UPrimaryDataAsset interface

public:
	// 本次体验要求激活的 Game Feature 插件列表（填插件名，运行时解析成插件 URL）
	// List of Game Feature Plugins this experience wants to have active
	UPROPERTY(EditDefaultsOnly, Category = Gameplay)
	TArray<FString> GameFeaturesToEnable;

	// 玩家默认使用的 Pawn 数据（决定角色类、能力集、摄像机等）
	/** The default pawn class to spawn for players */
	//@TODO: Make soft?
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TObjectPtr<const ULyraPawnData> DefaultPawnData;

	// 体验被 加载/激活/反激活/卸载 四个阶段时分别要执行的动作列表
	// List of actions to perform as this experience is loaded/activated/deactivated/unloaded
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	// 额外组合进来的动作集，用于把公共动作抽出来复用（例如「通用 HUD」）
	// List of additional action sets to compose into this experience
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<TObjectPtr<ULyraExperienceActionSet>> ActionSets;
};
