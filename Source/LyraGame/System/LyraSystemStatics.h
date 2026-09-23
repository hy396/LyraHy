// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * 通用静态工具函数库（蓝图可调用）：资产 ID 转换、切下一局、
 * 批量改材质参数、按类查找组件等。
 */
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/SoftObjectPtr.h"

#include "LyraSystemStatics.generated.h"

template <typename T> class TSubclassOf;

class AActor;
class UActorComponent;
class UObject;
struct FFrame;

// 蓝图函数库：与具体玩法无关的通用工具
UCLASS()
class ULyraSystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// 由主资源 ID 取软引用；即使资产没加载也能拿到
	/** Returns the soft object reference associated with a Primary Asset Id, this works even if the asset is not loaded */
	UFUNCTION(BlueprintPure, Category = "AssetManager", meta=(DeterminesOutputType=ExpectedAssetType))
	static TSoftObjectPtr<UObject> GetTypedSoftObjectReferenceFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId, TSubclassOf<UObject> ExpectedAssetType);

	// 由对外广告的体验名取对应的主资源 ID
	UFUNCTION(BlueprintCallable)
	static FPrimaryAssetId GetPrimaryAssetIdFromUserFacingExperienceName(const FString& AdvertisedExperienceID);

	// 切到下一局（仅在服务端可用）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Lyra", meta = (WorldContext = "WorldContextObject"))
	static void PlayNextGame(const UObject* WorldContextObject);

	// 给目标 Actor 上所有 Mesh 组件的所有分区设置标量材质参数
	// Sets ParameterName to ParameterValue on all sections of all mesh components found on the TargetActor
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta=(DefaultToSelf="TargetActor"))
	static void SetScalarParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const float ParameterValue, bool bIncludeChildActors = true);

	// 同上，设置向量参数
	// Sets ParameterName to ParameterValue on all sections of all mesh components found on the TargetActor
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta=(DefaultToSelf="TargetActor"))
	static void SetVectorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FVector ParameterValue, bool bIncludeChildActors = true);

	// 同上，设置颜色参数
	// Sets ParameterName to ParameterValue on all sections of all mesh components found on the TargetActor
	UFUNCTION(BlueprintCallable, Category = "Rendering|Material", meta=(DefaultToSelf="TargetActor"))
	static void SetColorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FLinearColor ParameterValue, bool bIncludeChildActors = true);

	// 查找目标 Actor（含子 Actor）上所有继承自指定类的组件
	// Gets all the components that inherit from the given class
	UFUNCTION(BlueprintCallable, Category = "Actor", meta=(DefaultToSelf="TargetActor", ComponentClass="/Script/Engine.ActorComponent", DeterminesOutputType="ComponentClass"))
	static TArray<UActorComponent*> FindComponentsByClass(AActor* TargetActor, TSubclassOf<UActorComponent> ComponentClass, bool bIncludeChildActors = true);
};
